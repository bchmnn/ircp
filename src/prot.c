#include "prot.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h> // usleep

#include "cc1200/utils.h"
#include "util/crypto.h"
#include "util/log.h"
#include "util/ringbuffer.h"

#include "config.h"

#define LOGGING_LEVEL INFO
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void free_message(message_t* message) {
	if (!message) return;
	if (message->msg) free(message->msg);
	free(message);
}

void free_serial_message(serial_message_t* message) {
	if (!message) return;
	free_message(message->msg);
	free(message);
}

void free_mstring(mstring_t* mstring) {
	if (!mstring) return;
	if (mstring->str) free(mstring->str);
	free(mstring);
}

void free_serial_mstring(serial_mstring_t* smstring) {
	if (!smstring) return;
	if (smstring->mstr) free_mstring(smstring->mstr);
	free(smstring);
}

const char* stages_str(stages_t stage) {
	switch (stage) {
	case CONNECT:     return "CONNECT";
	case CHATTING:    return "CHATTING";
	case INTERRUPTED: return "INTERRUPTED";
	case RECONNECT:   return "RECONNECT";
	default:          return "UNKNOWN";
	}
}

const char* client_mode_str(client_mode_t client_mode) {
	switch (client_mode) {
	case MASTER:  return "MASTER";
	case SERVANT: return "SERVANT";
	default:      return "UNKNOWN";
	}
}

void print_session(session_t* session) {
	printf("session: {\n");
	printf("    stage: %s\n", stages_str(session->stage));
	printf("    client_mode: %s\n", client_mode_str(session->client_mode));
	printf("    rssi_seed: %d\n", (int32_t) session->rssi_seed);
	printf("}\n");
}

message_t* parse_message(u_int32_t len, char* str) {

	if (len < 5) {
		LDEBG("Message to short\n");
		return NULL;
	}

	if (((u_int32_t) str[0]) >= MESSAGE_TYPE_MAX) {
		LDEBG("Message has invalid type\n");
		return NULL;
	}

	u_int32_t crc = crc32(str, len - 4);
	u_int32_t crc_given = *((u_int32_t*) (str + len - 4));

	if (crc != crc_given) {
		LDEBG("CRC was invalid: given: 0x%08x calc: 0x%08x\n", crc_given, crc);
		return NULL;
	}

	message_t* msg = malloc(sizeof(message_t));
	if (len > 5) {
		msg->msg = malloc(sizeof(char) * ((len - 5) + 1));
		memcpy(msg->msg, (str + 1), len - 5);
		msg->msg[len - 5] ='\0';
	} else {
		msg->msg = NULL;
	}

	msg->msg_len = len - 5;
	msg->type = (u_int32_t) str[0];
	msg->crc32 = crc;

	return msg;

}

serial_message_t* msg_to_serial_msg(message_t* msg) {
	if (!msg || !msg->msg)
		return NULL;
	if (msg->msg_len < 4) {
		LWARN("parse_serial_message requires at least 4 bytes\n");
		return NULL;
	}

	serial_message_t* smsg = malloc(sizeof(serial_message_t));
	smsg->serial = *((u_int32_t*) msg->msg);
	if (msg->msg_len > 4) {
		char* str = malloc(sizeof(char) * (msg->msg_len - 4 + 1));
		memcpy(str, (msg->msg + 4), msg->msg_len - 4);
		str[msg->msg_len - 4] = '\0';
		free(msg->msg);
		msg->msg = str;
		msg->msg_len = msg->msg_len - 4;
	} else {
		msg->msg = NULL;
		msg->msg_len = 0;
		free(msg->msg);
	}
	smsg->msg = msg;
	return smsg;
}


mstring_t* gen_message_str(message_type_t type, char* body, size_t len) {

	if (type >= MESSAGE_TYPE_MAX) {
		LDEBG("Invalid type\n");
		return NULL;
	}

	if (body == NULL && len > 0) {
		LDEBG("Lenght can't be greater zero when body is NULL\n");
		return NULL;
	}

	mstring_t* mstring = malloc(sizeof(mstring_t));
	mstring->str = malloc(sizeof(char) * (len + 6));

	mstring->str[0] = (u_int8_t) type;

	if (len) memcpy((mstring->str + 1), body, len);

	u_int32_t crc = crc32(mstring->str, len + 1);
	memcpy((mstring->str + len + 1), (char*) &crc, 4);

	mstring->str[len + 5] = '\0';
	mstring->len = len + 5;

	return mstring;
}

mstring_t* gen_serial_message_str(message_type_t type, u_int32_t serial, char* body, size_t len) {

	char* smessge_str = malloc(sizeof(char) * (len + 4));
	*((u_int32_t*) smessge_str) = serial;
	memcpy((smessge_str + 4), body, len);

	return gen_message_str(type, smessge_str, len + 4);

}

serial_mstring_t* mstr_to_serial_mstr(mstring_t* mstring) {

	if (!mstring || !mstring->str)
		return NULL;

	// <type:1><serial:4><crc32:4>
	if (mstring->len < 9) {
		LWARN("Len must be at least 9 to parse serial: <type:1><serial:4><crc32:4>\n");
		return NULL;
	}

	u_int32_t serial = *((u_int32_t*) (mstring->str + 1));
	serial_mstring_t* smstr = malloc(sizeof(serial_mstring_t));

	smstr->mstr = mstring;
	smstr->serial = serial;

	return smstr;

}

int32_t handshake(session_t* session, bool(*abort)(void*), void* abort_args) {

	reset_sesson(session);
	srandom(time(NULL));

	mstring_t* pkt_tx = NULL;

	while (!abort(abort_args)) {
		LTRAC("Sending HANDSHAKE\n");
		pkt_tx = gen_message_str(HANDSHAKE, NULL, 0);
		cc1200_tx(pkt_tx->str, pkt_tx->len);
		free_mstring(pkt_tx);

		LTRAC("Receiving HANDSHAKE response\n");
		cc1200_pkt_t* pkt_rx = cc1200_rx((random() % 100) + 10);
		if (pkt_rx && pkt_rx->len > 0) {
			message_t* msg = parse_message(pkt_rx->len, pkt_rx->pkt);
			if (!msg) {
				LDEBG("Received invalid msg\n");
				free_cc1200_pkt(pkt_rx);
				continue;
			} else if (msg->type == HANDSHAKE_ACK && msg->msg_len == 1) {
				LDEBG("Received HANDSHAKE_ACK\n");
				LDEBG("Meassured RSSI was: %d\n", (int32_t) pkt_rx->rssi);
				// TODO compare msg rssi with meassured rssi
				session->rssi_seed = (int8_t) *msg->msg;
				session->client_mode = MASTER;
			} else if (msg->type == HANDSHAKE) {
				LDEBG("Received HANDSHAKE\n");
				pkt_tx = gen_message_str(
						HANDSHAKE_ACK,
						(char*) &pkt_rx->rssi,
						1
				);
				cc1200_tx(pkt_tx->str, pkt_tx->len);
				free_mstring(pkt_tx);
				session->rssi_seed = pkt_rx->rssi;
				session->client_mode = SERVANT;
			}
			free_message(msg);
			free_cc1200_pkt(pkt_rx);
			session->stage = CHATTING;
			return 0;
		}
		if (pkt_rx)
			free_cc1200_pkt(pkt_rx);
	}
	return 1;
}


int32_t chat(
	session_t* session,
	bool(*abort)(void*),
	char*(read_buffer)(void*),
	void* func_args
) {

	rb_t* serial_tx_buf = rb_init(SERIAL_TX_BUF_SIZE, serial_mstring_t*, SINGLE_ELEMENT);

	u_int32_t serial_tx = 0;
	u_int32_t serial_rx = 0;
	bool nak_received = false;

	mstring_t* pkt_tx = NULL;

	while (!abort(func_args)) {
		if (nak_received || !rb_empty(serial_tx_buf)) { // RESEND/NAK
			serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
			cc1200_tx(smstr->mstr->str, smstr->mstr->len);
			nak_received = false;
		} else { // SEND MESSAGE
			char* str = read_buffer(func_args);
			if (str) {
				LTRAC("Starting trans \n");
				size_t len = strlen(str);
				pkt_tx = gen_serial_message_str(CHAT, serial_tx, str, len);
				cc1200_tx(pkt_tx->str, pkt_tx->len);
				serial_mstring_t* smstr = mstr_to_serial_mstr(pkt_tx);
				rb_push_ptr(serial_tx_buf, smstr);
				serial_tx++;
				free(str);
			}
		}

		LTRAC("Starting recv\n");
		cc1200_pkt_t* pkt_rx = cc1200_rx((random() % 100) + 10); //100
		LTRAC("Stopped recv\n");

		if (pkt_rx && pkt_rx->len > 0) {
			message_t* msg = parse_message(pkt_rx->len, pkt_rx->pkt);
			if (!msg) {
				LDEBG("Received invalid msg\n");
				free_cc1200_pkt(pkt_rx);
				cc1200_recover_err();
				continue;
			}
			if (msg->type == HANDSHAKE || msg->type == CIAO) {
				if (msg->type == CIAO)
					printf("-- your budy logged out.\n");
				LDEBG("Received HANDSHAKE/CIAO\n");
				update_rssi_avg(session, pkt_rx->rssi);
				session->stage = CONNECT;
				free_rb_deep_ptr(serial_tx_buf, (void(*)(void*)) &free_serial_mstring);
				free_message(msg);
				free_cc1200_pkt(pkt_rx);
				cc1200_recover_err();
				return 0;
			}
			serial_message_t* smsg = msg_to_serial_msg(msg);
			update_rssi_avg(session, pkt_rx->rssi);
			if (msg->type == CHAT_ACK) { // SENDER
				LDEBG("Received CHAT_ACK: serial: %u\n", smsg->serial);
				serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
				while (smstr && smstr->serial <= smsg->serial) {
					smstr = (serial_mstring_t*) rb_pop_ptr(serial_tx_buf);
					free_serial_mstring(smstr);
					smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
				}
			} else if (msg->type == CHAT_NAK) { // SENDER
				LDEBG("Received CHAT_NAK: serial: %u\n", smsg->serial);
				serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
				while (smstr && smstr->serial < smsg->serial) {
					smstr = (serial_mstring_t*) rb_pop_ptr(serial_tx_buf);
					free_serial_mstring(smstr);
					smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
				}
				nak_received = true;
			} else if (msg->type == CHAT) { // RECEIVER
				LDEBG("Received CHAT\n");
				if (smsg->serial > serial_rx) { // send NAK
					LDEBG("Serial to high: %u/%u\n", smsg->serial, serial_rx);
					LDEBG("Sending NAK: %u\n", serial_rx);
					pkt_tx = gen_serial_message_str(CHAT_NAK, serial_rx, NULL, 0);
					cc1200_tx(pkt_tx->str, pkt_tx->len);
					free_mstring(pkt_tx);
				} else if (smsg->serial < serial_rx) { // send ACK
					LDEBG("Serial lower: %u/%u\n", smsg->serial, serial_rx);
					LDEBG("Sending ACK: %u\n", serial_rx - 1);
					pkt_tx = gen_serial_message_str(CHAT_ACK, serial_rx - 1, NULL, 0);
					cc1200_tx(pkt_tx->str, pkt_tx->len);
					free_mstring(pkt_tx);
				} else { // print and send ACK
					LDEBG("Serial valid: %u/%u\n", smsg->serial, serial_rx);
					LDEBG("Sending ACK: %u\n", serial_rx);
					if (smsg->msg->msg)
						printf("budy: %s\n", smsg->msg->msg);
					pkt_tx = gen_serial_message_str(CHAT_ACK, serial_rx, NULL, 0);
					cc1200_tx(pkt_tx->str, pkt_tx->len);
					free_mstring(pkt_tx);
					serial_rx++;
				}
			}
			free_serial_message(smsg);
		}
		if (pkt_rx)
			free_cc1200_pkt(pkt_rx);

		cc1200_recover_err();

	}

	pkt_tx = gen_message_str(CIAO, NULL, 0);
	reset_sesson(session);
	cc1200_tx(pkt_tx->str, pkt_tx->len);
	free_mstring(pkt_tx);

	return 0;

}
