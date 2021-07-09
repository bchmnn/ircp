#include "cc1200/utils.h"

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>

#include "cc1200/control.h"
#include "cc1200/regs.h"
#include "util/log.h"

#define LOGGING_LEVEL DEBUG
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

cc1200_pkt_t* malloc_cc1200_pkt(u_int8_t len) {
	cc1200_pkt_t* pkt = malloc(sizeof(cc1200_pkt_t));
	pkt->len = len;
	pkt->pkt = malloc(sizeof(char)*(len+1));
	return pkt;
}

void free_cc1200_pkt(cc1200_pkt_t* pkt) {
	if(!pkt->pkt) free(pkt->pkt);
	if(!pkt) free(pkt);
}

bool cc1200_wait_till_mode(int32_t mode, size_t timeout_ms) {
	int32_t curr_mode = -1;
	size_t wait_us = 10;
	size_t waited_us = 0;
	if (timeout_ms == 0)
		timeout_ms = 1000;
	while (mode != curr_mode) {
		if (timeout_ms < waited_us / 1000)
			return true;
		usleep(wait_us);
		waited_us += wait_us;
		curr_mode = cc1200_get_status_snop();
		LTRAC("CC1200: curr mode: %s\n", get_status_cc1200_str());
	}
	return false;
}

bool cc1200_wait_till_bytes_in_fifo(size_t timeout_ms) {
	size_t wait_us = 10;
	size_t waited_us = 0;
	while (!cc1200_reg_read(NUM_RXBYTES, 0)) {
		if (timeout_ms < waited_us / 1000)
			return true;
		usleep(wait_us);
		waited_us += wait_us;
	}
	return false;
}

cc1200_pkt_t* cc1200_rx(size_t timeout) {
	// struct timeval start, stop;
	cc1200_pkt_t*  pkt = NULL;
	int            cnt = 0;
	int            wait = timeout ? timeout : 100;

	cc1200_cmd(SRX);
	cc1200_wait_till_mode(RX, 1000);

	while (wait) {
		LTRAC("Waiting for pkt\n");
		if (!cc1200_reg_read(NUM_RXBYTES, 0)) {
			usleep(10);
			wait--;
			continue;
		}

		int pkt_len = cc1200_reg_read(RXFIFO, 0);
		LDEBG("Receiving pkt with len: %d\n", pkt_len);

		wait = 100;
		usleep(10 * pkt_len * 3);
		while (cc1200_reg_read(NUM_RXBYTES, 0) < pkt_len) {
			if (!wait) {
				LWARN("Timeout for pkt\n");
				return NULL;
			}
			usleep(10 * pkt_len * 3);
			wait--;
		}

		pkt = malloc_cc1200_pkt(pkt_len);
		char c;

		while (pkt_len) {
			c = cc1200_reg_read(RXFIFO, 0);
			*(pkt->pkt+(cnt++)) = c;
			pkt_len--;
		}

		*(pkt->pkt+cnt) = '\0';

		wait = 100;
		while (cc1200_reg_read(NUM_RXBYTES, 0) < 2) {
			if (!wait) {
				LWARN("Timeout for CRC16\n");
				free_cc1200_pkt(pkt);
				return NULL;
			}
			usleep(10);
			wait--;
		}

		for (size_t i = 0; i < CRC16; i++) {
			c = cc1200_reg_read(RXFIFO, 0);
			*(((char*) &pkt->rssi)+i) = c;
		}

		LDEBG("Received pkt: CRC16: { rssi: %d, link_quality: %d, crc_status: %s }\n",
			pkt->rssi,
			pkt->link_quality,
			pkt->crc_status ? "\"valid\"" : "\"error\""
		);

		break;
	}

	return pkt;
}

void cc1200_tx(char* packet, int len) {
	if (len > 127) {
		LERR("Packet len max 127: instead %d\n", len);
		return;
	}
	cc1200_write_tx_fifo(packet, len);
	cc1200_cmd(STX);
	cc1200_wait_till_mode(IDLE, 1000);
}

void cc1200_recover_err() {
	u_int32_t mode = cc1200_get_status_snop();
	if (mode == RX_FIFO_ERROR) {
		LWARN("CC1200: Recovering RX_FIFO_ERROR\n");
		cc1200_cmd(SFRX);
		cc1200_wait_till_mode(IDLE, 1000);
	} else if (mode == TX_FIFO_ERROR) {
		LWARN("CC1200: Recovering TX_FIFO_ERROR\n");
		cc1200_cmd(SFTX);
		cc1200_wait_till_mode(IDLE, 1000);
	}
}