#include "cc1200_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <external/SPIv1.h>

#include "util/log.h"
#include "cc1200_reg.h"

#define LOGGING_LEVEL DEBUG
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

cc1200_pkt_t* malloc_cc1200_pkt(u_int8_t len) {
	cc1200_pkt_t* pkt = malloc(sizeof(cc1200_pkt_t));
	pkt->len = len;
	pkt->pkt = malloc(sizeof(char)*len);
	return pkt;
}

void free_cc1200_pkt(cc1200_pkt_t* pkt) {
	free(pkt->pkt);
	free(pkt);
}

int freg_adr [NUM_FREQ_REGS] = {FREQ2,FREQ1, FREQ0,FS_CHP, FS_VCO4,FS_VCO2};

REG_TYPE freq [NUM_FREQ][NUM_FREQ_REGS] = {
	{
		{ FREQ2,   0x5b },
		{ FREQ1,   0x80 },
		{ FREQ0,   0x00 },
		{ FS_CHP,  0x2a },
		{ FS_VCO4, 0x11 },
		{ FS_VCO2, 0x64 }
	}, {
		{ FREQ2,   0x56 },
		{ FREQ1,   0xcc },
		{ FREQ0,   0xcc },
		{ FS_CHP,  0x2b },
		{ FS_VCO4, 0x11 },
		{ FS_VCO2, 0x4c }
	}
};

int cc1200_init() {
	// first initialize
	printf("INFO: Initializing SPI\n");
	if (spi_init()) {
		printf("ERR: Initialization failed\n");
		return -1;
	}
	// reset CC1200
	// cc1200 is now in idle mode, registers have their default values
	cc1200_cmd(SRES);
	return 0;
}

void cc1200_init_reg(REG_TYPE* RegSettings, REG_TYPE* ExtRegSettings) {
	cc1200_init();
	// reprogram the registers
	cc1200_regs_write(RegSettings, MAX_REG);
	cc1200_regs_write(ExtRegSettings, MAX_EXT_REG);
	// get status information
	// SNOP command has to be executed prior to status retrieval
	cc1200_cmd(SNOP);
	printf("INFO: Status: %s\n", get_status_cc1200_str());
}

void set_mode(int value, int len) {
	int pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
	pkt_cfg0 |= value;
	cc1200_reg_write(PKT_CFG0, value);
	cc1200_reg_write(PKT_LEN, len);
}

void set_preamble(int value) {
	cc1200_reg_write(PREAMBLE_CFG1, value);
}

void set_symbole_rate(int value) {
	cc1200_reg_write(SYMBOL_RATE2, value);
}

void set_freq(REG_TYPE* freq, int len) {
	for (int i = 0; i < len ;i++) {
		int adr = freq[i].adr;
		int val = freq[i].val;
		cc1200_reg_write(adr, val);
	}
}

void set_freg_reg(REG_TYPE* freq, int* value, int len) {
	for (int i = 0; i < len ;i++) {
		freq[i].adr = freg_adr[i];
		freq[i].val = value[i];
	}
}

void get_freg_reg(REG_TYPE* freq, int* value, int len) {
	for (int i = 0; i < len ;i++) {
		value[i] = freq[i].val ;
	}
}


/*
 * Returns: 0 on success; 1 on timeout
 */
int wait_till_mode(int mode, int timeout_ms, bool exit_on_timeout) {
	int curr_mode = -1;
	int wait_us = 10;
	int waited_us = 0;
	if (timeout_ms < 0)
		timeout_ms = 1000;
	while (mode != curr_mode) {
		if (timeout_ms < waited_us / 1000) {
			if (exit_on_timeout) {
				printf("Err: Wait timed out\n");
				spi_shutdown();
				exit(1);
			}
			return 1;
		}
		usleep(wait_us);
		waited_us += wait_us;
		cc1200_cmd(SNOP);
		curr_mode = get_status_cc1200();
		LTRAC("CC1200: curr mode: %s\n", get_status_cc1200_str());
	}
	return 0;
}

int wait_till_bytes_in_queue(int timeout_ms, bool exit_on_timeout) {
	int wait_us = 10;
	int waited_us = 0;
	while (!cc1200_reg_read(NUM_RXBYTES, 0)) {
		if (timeout_ms < waited_us / 1000) {
			if (exit_on_timeout) {
				printf("Err: Wait timed out\n");
				spi_shutdown();
				exit(1);
			}
			return 1;
		}
		usleep(wait_us);
		waited_us += wait_us;
	}
	return 0;
}

void gen_random_massage(char *s, const int len) {
	static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len] = 0;
}

void write_tx_fifo(char* data, int len) {

	cc1200_reg_write(TXFIFO , len);
	data[len] = 0;
	while (*data != '\0') {
		int c = *data;
		cc1200_reg_write(TXFIFO , c);
		//printf("DEBUG: int: %d , char:  %c \n", c, c);
		usleep(MS_IN_U(10));
		data++;
	}

}

int cc1200_rx_preparar() {

	set_mode(MODE_VARIABLE_LENGTH, PKT_LEN_VARIABLE_MODE);

	// printf("INFO: Putting into receive mode...\n");
	// receive mode
	cc1200_cmd(SRX);
	wait_till_mode(RX, 1000, false);

	cc1200_cmd(SNOP);
	// printf("INFO: Status: %s\n", get_status_cc1200_str());
	
	return cc1200_reg_read(PKT_LEN, 0);
}

cc1200_pkt_t* cc1200_rx() {

	// struct timeval start, stop;
	cc1200_pkt_t*  pkt = NULL;
	int            cnt = 0;
	int            wait = 1000;

	while (wait) {
		LTRAC("Waiting for pkt\n");
		if (!cc1200_reg_read(NUM_RXBYTES, 0)) {
			usleep(10);
			wait--;
			continue;
		}

		int pkt_len = cc1200_reg_read(RXFIFO, 0);
		LTRAC("Receiving pkt with len: %d\n", pkt_len);

		wait = 1000;
		usleep(10 * pkt_len * 3);
		while (cc1200_reg_read(NUM_RXBYTES, 0) < pkt_len) {
			if (!wait) {
				LWARN("Timeout for pkt\n");
				return NULL;
			}
			usleep(10 * pkt_len * 3);
			wait--;
		}

		pkt = malloc_cc1200_pkt(pkt_len+1);
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

		LTRAC("Received pkt had RSSI: %d\n", pkt->rssi);
		LTRAC("Received pkt had link_quality: %d\n", pkt->link_quality);
		LTRAC("Received pkt had crc_status: %s\n", pkt->crc_status ? "Valid" : "Error");

		break;
	}

	return pkt;
}

void cc1200_tx(char* packet, int len) {
	if (len > 256) {
		printf("ERROR: packet len max 256: instead %d\n", len);
		return;
	}

	set_mode(MODE_VARIABLE_LENGTH, PKT_LEN_VARIABLE_MODE);
	write_tx_fifo(packet, len);

	cc1200_cmd(STX);

	wait_till_mode(IDLE, 1000, false);
}

void cc1200_recover_err() {
	cc1200_cmd(SNOP);
	int mode = get_status_cc1200();
	if (mode == RX_FIFO_ERROR) {
		LWARN("CC1200: Recovering RX_FIFO_ERROR\n");
		cc1200_cmd(SFRX);
		wait_till_mode(IDLE, 1000, false);
	} else if (mode == TX_FIFO_ERROR) {
		LWARN("CC1200: Recovering TX_FIFO_ERROR\n");
		cc1200_cmd(SFTX);
		wait_till_mode(IDLE, 1000, false);
	}
}
