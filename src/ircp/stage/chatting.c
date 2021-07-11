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

int32_t chat(
	session_t* session,
	boolfunc_t abort,
	void* abort_args,
	strfunc_t readln,
	void* readln_args
) {

	rb_t* serial_tx_buf = rb_init(
		SERIAL_TX_BUF_SIZE,
		serial_mstring_t*,
		SINGLE_ELEMENT
	);

	u_int32_t serial_tx = 0;
	u_int32_t serial_rx = 0;
	bool nak_received = false;

	mstring_t* pkt_tx = NULL;

	while (!abort(abort_args)) {
		if (nak_received || !rb_empty(serial_tx_buf)) { // RESEND/NAK
			serial_mstring_t* smstr = (serial_mstring_t*) rb_peek_ptr(serial_tx_buf);
			cc1200_tx(smstr->mstr->str, smstr->mstr->len);
			nak_received = false;
		} else { // SEND MESSAGE
			char* str = readln(readln_args);
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
	reset_session(session);
	cc1200_tx(pkt_tx->str, pkt_tx->len);
	free_mstring(pkt_tx);

	return 0;

}


