#include "ircp/stage/chatting.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cc1200/utils.h"
#include "ircp/utils.h"
#include "util/types.h"
#include "util/log.h"
#include "util/ringbuffer.h"
#include "config.h"

#define LOGGING_LEVEL INFO
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

static rb_t* SERIAL_TX_BUF = NULL;
static u_int32_t serial_tx = 0;
static u_int32_t serial_rx = 0;

void resend_from_buf() {
	if (rb_empty(SERIAL_TX_BUF))
		return;
	serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(SERIAL_TX_BUF);
	cc1200_tx(smstr->mstr->str, smstr->mstr->len);
}

void send_from_stdin(strfunc_t readln, void* readln_args) {
	char* str = readln(readln_args);
	if (!str)
		return;
	LTRAC("Starting trans\n");
	size_t len = strlen(str);
	mstring_t* pkt_tx = gen_serial_message_str(CHAT, serial_tx, str, len);
	cc1200_tx(pkt_tx->str, pkt_tx->len);
	serial_mstring_t* smstr = mstr_to_serial_mstr(pkt_tx);
	rb_push_ptr(SERIAL_TX_BUF, smstr);
	serial_tx++;
	free(str);
}

void handle_handshake(session_t* session) {
	LDEBG("Received HANDSHAKE\n");
	session->stage = CONNECT;
	free_rb_deep_ptr(SERIAL_TX_BUF, (routine_t) &free_serial_mstring);
	SERIAL_TX_BUF = NULL;
	serial_tx = 0;
	serial_rx = 0;
}

void handle_ciao(session_t* session) {
	printf("-- your budy logged out.\n");
	LDEBG("Received CIAO\n");
	session->stage = CONNECT;
	free_rb_deep_ptr(SERIAL_TX_BUF, (routine_t) &free_serial_mstring);
	SERIAL_TX_BUF = NULL;
	serial_tx = 0;
	serial_rx = 0;
}

void handle_chat_ack(serial_message_t* smsg) {
	if (!smsg) {
		LWARN("NULL was passed\n");
		return;
	}

	LDEBG("Received CHAT_ACK: serial: %u\n", smsg->serial);
	serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(SERIAL_TX_BUF);
	while (smstr && smstr->serial <= smsg->serial) {
		smstr = (serial_mstring_t*) rb_pop_ptr(SERIAL_TX_BUF);
		free_serial_mstring(smstr);
		smstr = (serial_mstring_t*) rb_peek_ptr(SERIAL_TX_BUF);
	}
}

void handle_chat_nak(serial_message_t* smsg) {
	if (!smsg) {
		LWARN("NULL was passed\n");
		return;
	}

	LDEBG("Received CHAT_NAK: serial: %u\n", smsg->serial);
	serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(SERIAL_TX_BUF);
	while (smstr && smstr->serial < smsg->serial) {
		smstr = (serial_mstring_t*) rb_pop_ptr(SERIAL_TX_BUF);
		free_serial_mstring(smstr);
		smstr = (serial_mstring_t*) rb_peek_ptr(SERIAL_TX_BUF);
	}
}

void handle_chat(serial_message_t* smsg) {
	if (!smsg) {
		LWARN("NULL was passed\n");
		return;
	}

	LDEBG("Received CHAT\n");
	mstring_t* pkt_tx = NULL;

	if (smsg->serial > serial_rx) { // send NAK
		LDEBG("Serial to high: %u/%u\n", smsg->serial, serial_rx);
		LDEBG("Sending NAK: %u\n", serial_rx);
		pkt_tx = gen_serial_message_str(CHAT_NAK, serial_rx, NULL, 0);
	} else if (smsg->serial < serial_rx) { // send ACK
		LDEBG("Serial lower: %u/%u\n", smsg->serial, serial_rx);
		LDEBG("Sending ACK: %u\n", serial_rx - 1);
		pkt_tx = gen_serial_message_str(CHAT_ACK, serial_rx - 1, NULL, 0);
	} else { // print and send ACK
		LDEBG("Serial valid: %u/%u\n", smsg->serial, serial_rx);
		if (smsg->msg->msg)
			printf("budy: %s\n", smsg->msg->msg);
		LDEBG("Sending ACK: %u\n", serial_rx);
		pkt_tx = gen_serial_message_str(CHAT_ACK, serial_rx, NULL, 0);
		serial_rx++;
	}

	if (pkt_tx) {
		cc1200_tx(pkt_tx->str, pkt_tx->len);
		free_mstring(pkt_tx);
	}
}

int32_t chat(
	session_t* session,
	boolfunc_t abort, void* abort_args,
	strfunc_t readln, void* readln_args
) {

	if (!SERIAL_TX_BUF) {
		SERIAL_TX_BUF = rb_init(
			SERIAL_TX_BUF_SIZE,
			serial_mstring_t*,
			SINGLE_ELEMENT
		);
	}

	bool ret_received = false;
	bool nak_received = false;
	u_int32_t resend_timer = 0;
	mstring_t* pkt_tx = NULL;

	while (!abort(abort_args)) {

		if (nak_received || resend_timer == 0) {
			resend_from_buf();
			nak_received = false;
			resend_timer = RESEND_AFTER_ITERATIONS;
		} else { // SEND MESSAGE
			send_from_stdin(readln, readln_args);
		}

		resend_timer--;

		LTRAC("Starting recv\n");
		int8_t rssi = RSSI_INVALID;
		cc1200_pkt_t* pkt_rx = cc1200_rx((random() % 100) + 10, &rssi);

		if (rssi != RSSI_INVALID && session->rssi_idle == RSSI_INVALID) {
			session->rssi_idle = rssi;
		}

		u_int8_t interference_score = calc_interference_score(session, rssi);
		if (rssi != RSSI_INVALID && interference_score > RSSI_TOLERANCE) {
			LWARN("Interference likely, RSSI: %d, Interference Score: %u\n", (int32_t) rssi, (u_int32_t) interference_score);
			print_session(session);
		}

		if (!pkt_rx && rssi != RSSI_INVALID)
			update_rssi_idle(session, rssi);

		if (pkt_rx && pkt_rx->len > 0) {
			message_t* msg = parse_message(pkt_rx->len, pkt_rx->pkt);
			if (!msg) {
				LDEBG("Received invalid msg\n");
				free_cc1200_pkt(pkt_rx);
				cc1200_recover_err();
				continue;
			}

			serial_message_t* smsg = msg_to_serial_msg(msg);
			update_rssi_high(session, pkt_rx->rssi);

			switch (msg->type) {
				case HANDSHAKE:
					handle_handshake(session);
					ret_received = true;
					break;
				case CIAO:
					handle_ciao(session);
					ret_received = true;
					break;
				case CHAT_ACK:
					handle_chat_ack(smsg);
					break;
				case CHAT_NAK:
					handle_chat_nak(smsg);
					nak_received = true;
					break;
				case CHAT:
					handle_chat(smsg);
					resend_timer = RESEND_AFTER_ITERATIONS;
					break;
				default:
					break;
			}
			if (smsg) free_serial_message(smsg);
		}

		if (pkt_rx) free_cc1200_pkt(pkt_rx);
		cc1200_recover_err();
		if (ret_received) return 0;

	}

	pkt_tx = gen_message_str(CIAO, NULL, 0);
	reset_session(session);
	cc1200_tx(pkt_tx->str, pkt_tx->len);
	free_mstring(pkt_tx);

	return 0;

}
