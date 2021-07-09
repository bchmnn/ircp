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

#define LOGGING_LEVEL DEBUG
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

void free_mstring(mstring_t* mstring) {
	if (!mstring) return;
	if (mstring->str) free(mstring->str);
	free(mstring);
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
		msg->msg = malloc(sizeof(char) * ((len - 5)+1));
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

mstring_t* message_str(message_type_t type, char* body, size_t len) {

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


int32_t handshake(session_t* session, bool(*abort)(void*), void* abort_args) {

	reset_sesson(session);
	srandom(time(NULL));

	mstring_t* pkt_tx = NULL;

	while (!abort(abort_args)) {
		LTRAC("Sending HANDSHAKE\n");
		pkt_tx = message_str(HANDSHAKE, NULL, 0);
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
				pkt_tx = message_str(
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

	mstring_t* pkt_tx = NULL;

	while (!abort(func_args)) {

		char* str = read_buffer(func_args);
		if(str) {
			LTRAC("[CHAT]CC1200: Starting trans \n");
			size_t len = strlen(str);
			pkt_tx = message_str(CHAT,str,len);
			cc1200_tx(pkt_tx->str, pkt_tx->len);
			free_mstring(pkt_tx);
			free(str);
			LTRAC("[CHAT]CC1200: Stopped trans\n");
		}
		LTRAC("[CHAT]CC1200: Starting recv\n");
		cc1200_pkt_t* pkt_rx = cc1200_rx((random() % 100) + 10); //100
		LTRAC("[CHAT]CC1200: Stopped recv\n");
		if (pkt_rx && pkt_rx->len > 0) {
			message_t* msg = parse_message(pkt_rx->len, pkt_rx->pkt);
			if (!msg) {
				if(msg->msg) printf("%s\n",msg->msg);
				LDEBG("Received invalid msg\n");
				free_cc1200_pkt(pkt_rx);
				continue;
			}
			// int8_t rssi = pkt_rx->rssi;
			// if(rssi > session->rssi_avg- RSSI_TOLERANCE || rssi < session->rssi_avg + RSSI_TOLERANCE ){
			// 	printf("Interupted\n");
			// 	LTRAC("Receiving CHAT INTERRUPTED\n");
			// 	session->stage = INTERRUPTED;
			// 	free_message(msg);
			// 	free_cc1200_pkt(pkt_rx);
			// 	return 0;
			// }
			if (msg->type == HANDSHAKE || msg->type == CIAO)  {
				printf("Handshake\n");
				LTRAC("Receiving CHAT Handshake or ciao\n");
				update_rssi_avg(session, pkt_rx->rssi );
				session->stage = CONNECT;
				free_message(msg);
				free_cc1200_pkt(pkt_rx);
				return 0;
			} else if (msg->type == CHAT_ACK && msg->msg_len == 1) {
				LTRAC("Receiving CHAT_ACK\n");
				update_rssi_avg(session, pkt_rx->rssi );
			} else if (msg->type == CHAT) {
				LTRAC("Receiving CHAT \n");
				if(msg->msg)
					printf("%s\n",msg->msg);
				update_rssi_avg(session, pkt_rx->rssi );
				pkt_tx = message_str(CHAT_ACK,(char*) &pkt_rx->rssi,1);
				cc1200_tx(pkt_tx->str, pkt_tx->len);
				free_mstring(pkt_tx);
			}

			free_message(msg);
			free_cc1200_pkt(pkt_rx);
			cc1200_recover_err();
		}
	}

	pkt_tx = message_str(CIAO,NULL,0);
	reset_sesson(session);
	cc1200_tx(pkt_tx->str, pkt_tx->len);
	free_mstring(pkt_tx);

	return 0;

}
