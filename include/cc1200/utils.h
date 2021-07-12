#ifndef CC1200_UTILS_H
#define CC1200_UTILS_H

#include <stdlib.h>
#include <stdbool.h>

#define CRC16 2
#define RSSI_INVALID -128

typedef struct {
	u_int8_t len;
	char* pkt;
	int8_t rssi;              // CRC16[0]
	u_int8_t link_quality: 7; // CRC16[1][0:6]
	u_int8_t crc_status: 1;   // CRC16[1][7]
} cc1200_pkt_t;

/**
 * Allocates memory for cc1200_pkt_t
 * NOTE!: allocates len+1 for potential null terminator
 * @param len  len of underlying char* pkt to allocate
 */
cc1200_pkt_t* malloc_cc1200_pkt(u_int8_t len);

/**
 * Frees pkt struct and fields;
 */
void free_cc1200_pkt(cc1200_pkt_t* pkt);

/**
 * Blocking wait until mode is reached by the CC1200
 * chip.
 * @param mode        mode to wait for
 * @param timeout_ms  abort after ms
 * @return            true on timeout, false else
 */
bool cc1200_wait_till_mode(int32_t mode, size_t timeout_ms);

/**
 * Blocking wait until atleast 1 byte is present in
 * the FIFO queue of CC1200.
 * @param timeout_ms  abort after ms
 * @return            true on timeout, false else
 */
bool cc1200_wait_till_bytes_in_fifo(size_t timeout_ms);

void cc1200_tx(char* packet, int len);

/**
 * @brief Receive a packet with the CC1200
 * Receive a packet with timeout. Also RSSI
 * is meassured.
 * @param timeout  wait for timeout*10 us (default: 100*10 us)
 * @param rssi     a int8_t pointer whose value is set to
 *                 meassured rssi
 * @return         cc1200_pkt_t pointer on success, NULL on err
 */
cc1200_pkt_t* cc1200_rx(size_t timeout, int8_t* rssi);

void cc1200_recover_err();

#endif //CC1200_UTILS_H