#include "cc1200_function.h"

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

void set_mode(int value,int len) {
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


char* cc1200_rx() {

	int cnt = 0;
	char* buf = malloc(sizeof(char)*(256 + CRC16));

	int wait = 1000;

	while (wait) {
		if (!cc1200_reg_read(NUM_RXBYTES,0)) {
			usleep(10);
			wait--;
			continue;
		}

		int pkt_len = cc1200_reg_read(RXFIFO, 0) + CRC16;
		while (pkt_len) {
			if (!cc1200_reg_read(NUM_RXBYTES, 0)) {
				continue;
			}
			buf[cnt++] = cc1200_reg_read(RXFIFO, 0);
			pkt_len--;
		}
		// TODO crc is truncated. Make struct instead: { msg, crc }
		buf[cnt-2] = '\0';
		break;
	}

	buf[cnt] = '\0';
	return buf;
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