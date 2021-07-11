#include "ircp/stage/connect.h"

#include <stdlib.h>
#include <time.h>

#include "cc1200/utils.h"
#include "ircp/utils.h"
#include "util/log.h"

#define LOGGING_LEVEL INFO
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

int32_t handshake(session_t* session, boolfunc_t abort, void* abort_args) {
	reset_session(session);
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
