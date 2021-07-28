#include "cc1200/control.h"

#include <stdlib.h>
#include <unistd.h>

#include <external/SPIv1.h>

#include "cc1200/regs.h"
#include "util/assert.h"

void cc1200_set_packet_mode(cc1200_packet_mode_t mode, u_int8_t len) {
	u_int8_t pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
	pkt_cfg0 &= ~CC1200_PKT_CFG0_MODE_MASK;
	pkt_cfg0 |= mode;
	cc1200_reg_write(PKT_CFG0, pkt_cfg0);
	cc1200_reg_write(PKT_LEN,  len);
}

cc1200_packet_mode_t cc1200_get_packet_mode() {
	u_int8_t pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
	return pkt_cfg0 & CC1200_PKT_CFG0_MODE_MASK;
}

int32_t cc1200_write_tx_fifo(char* buf, size_t len) {
	cc1200_packet_mode_t pkt_mode = cc1200_get_packet_mode();
	u_int8_t             pkt_len  = cc1200_get_packet_len();

	ASSERTF(pkt_mode == VAR_LENGTH, "Currently only pkt mode VAR_LENGTH is supported\n");

	if (len > pkt_len)
		return 1;

	if (pkt_mode == VAR_LENGTH)
		cc1200_reg_write(TXFIFO, len);

	for (size_t i = 0; i < len; i++) {
		cc1200_reg_write(TXFIFO, *buf++);
		usleep(10);
	}

	return 0;
}
