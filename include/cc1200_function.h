#ifndef CC1200_FUNCTION_H
#define CC1200_FUNCTION_H

#include <stdlib.h>
#include <stdbool.h>

#include "cc1200_reg.h"

#define MS_IN_U(ms) (ms * 1000)

#define MODE_FIXX_LENGTH               (1 << 3)
#define MODE_VARIABLE_LENGTH           (1 << 5)

#define PKT_LEN_VARIABLE_MODE          127

#define SYMBOLE_RATE_SLOW              0b00111111
#define SYMBOLE_RATE_MIDDLE            0b10001111 
#define SYMBOLE_RATE_FAST              0b11001111

#define PRAEMBLE_HIGE                  0b00110100      // 30                  
#define PREAMLE_MIDDLE                 0b00010100      // 20 <- default
#define PREAMLE_SLOW                   0b00001010      // 10

#define NUM_FREQ 2
#define NUM_FREQ_REGS 6
#define CRC16 2

typedef struct {
	u_int8_t len;
	char* pkt;
	int8_t rssi;              // CRC16[0]
	u_int8_t link_quality: 7; // CRC16[1][0:6]
	u_int8_t crc_status: 1;   // CRC16[1][7]
} cc1200_pkt_t;

/**
 * Allocates memory for cc1200_pkt_t
 * @param len  len of underlying char* pkt to allocate
 */
cc1200_pkt_t* malloc_cc1200_pkt(u_int8_t len);

/**
 * Frees pkt struct and fields;
 */
void free_cc1200_pkt(cc1200_pkt_t* pkt);


int cc1200_init();
void cc1200_init_reg(REG_TYPE* RegSettings, REG_TYPE* ExtRegSettings);
void set_mode(int value,int len);
void set_preamble(int value);
void set_symbole_rate(int value);
void set_freq( REG_TYPE* fre, int len);
void set_freg_reg(REG_TYPE* fre, int* value, int len);
void get_freg_reg(REG_TYPE* fre, int* value, int len);
int wait_till_mode(int mode, int timeout_ms, bool exit_on_timeout);
int wait_till_bytes_in_queue(int timeout_ms, bool exit_on_timeout);

void cc1200_tx(char* packet, int len);
cc1200_pkt_t* cc1200_rx();
int cc1200_rx_preparar();

void cc1200_recover_err();

#endif //CC1200_FUNCTION_H