#ifndef CC1200_CONTROL_H
#define CC1200_CONTROL_H

#include <stdlib.h>
#include <external/SPIv1.h>

#include "cc1200/regs.h"

static inline u_int32_t cc1200_get_status_snop() {
	cc1200_cmd(SNOP);
	return get_status_cc1200();
}

static inline const char* cc1200_get_status_str_snop() {
	cc1200_cmd(SNOP);
	return get_status_cc1200_str();
}

/**
 * Writes an array of register onto the
 * CC1200.
 * @param regs  array of addr:val
 * @param len   length of this array
 */
static inline void cc1200_regs_write(cc1200_reg_t* regs, size_t len) {
	for (size_t i = 0; i < len; i++)
		cc1200_reg_write(regs[i].adr, regs[i].val);
}

#define CC1200_PKT_CFG0_MODE_MASK 0b01100000

typedef enum {
	FIXED_LENGTH = 0,
	VAR_LENGTH   = 0b00100000,
	INF_LENGTH   = 0b01000000, // not implemented
	VAR_LENGTH_5 = 0b01100000  // not implemented
} cc1200_packet_mode_t;

/**
 * Sets the registers of the CC1200 to
 * send packets either in fixed or
 * variable length mode.
 * Warning: infinite length not implemented
 * @param mode  mode to set
 * @param len   (max) length of packets
 */
void cc1200_set_packet_mode(cc1200_packet_mode_t mode, u_int8_t len);

cc1200_packet_mode_t cc1200_get_packet_mode();

inline u_int8_t cc1200_get_packet_len() {
	return cc1200_reg_read(PKT_LEN, 0);
}

typedef enum {
	NO_PREAMBLE = 0,
	BYTE_0p5    = 0b0001,
	BYTE_1      = 0b0010,
	BYTE_1p5    = 0b0011,
	BYTE_2      = 0b0100,
	BYTE_3      = 0b0101,
	BYTE_4      = 0b0110,
	BYTE_5      = 0b0111,
	BYTE_6      = 0b1000,
	BYTE_7      = 0b1001,
	BYTE_8      = 0b1010,
	BYTE_12     = 0b1011,
	BYTE_24     = 0b1100,
	BYTE_30     = 0b1101,
} cc1200_num_preamble_t;

typedef enum {
	HEX_AA = 0b00,
	HEX_55 = 0b01,
	HEX_33 = 0b10,
	HEX_CC = 0b11
} cc1200_preamble_word_t;

// cc1200_setpreamble
inline void cc1200_set_preamble(
	cc1200_num_preamble_t num_preamb,
	cc1200_preamble_word_t preamb_word
) {
	cc1200_reg_write((num_preamb << 2) | preamb_word, 0);
}

// TODO
void cc1200_set_symbol_rate(u_int8_t val);

/**
 * Writes buf to TXFIFO.
 * @param buf
 * @param len  length of buffer
 * @return     0 on success, 1 no error (length to high)
 */
int32_t cc1200_write_tx_fifo(char* buf, size_t len);

#endif //CC1200_CONTROL_H