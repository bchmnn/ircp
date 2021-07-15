#include "prog/prssi.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <external/SPIv1.h>

#include "cc1200/config.h"
#include "cc1200/control.h"
#include "cc1200/freq.h"
#include "cc1200/utils.h"
#include "util/log.h"
#include "util/types.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void prssi(prssi_mode_t mode, u_int32_t freq, boolfunc_t abort, void* args) {

	if (abort == NULL) {
		LWARN("Parameter abort is NULL, program won't terminate\n");
	}

	if (spi_init()) {
		LERR("Initialization failed\n");
		return;
	}

	cc1200_cmd(SRES);
	if (cc1200_wait_till_mode(IDLE, 1000)) {
		LERR("Reset timed out\n");
		return;
	}

	cc1200_regs_write(CC1200_BASE_CONFIG_RSSI, CC1200_NUM_REGS);
	freq = freq == 0 ? DEFAULT_FREQUENCY : freq;
	LINFO("Sending on frequency %u MHz\n", freq);
	if (cc1200_set_freq(freq)) {
		LERR("Could not set frequency\n");
		return;
	}

	switch (mode) {
	case PRSSI_RX:
		cc1200_cmd(SRX);
		int8_t rssi = 0;
		while (!abort || !abort(args)) {
			rssi = (int8_t) cc1200_reg_read(RSSI1, 0);
			printf("RSSI: %d\n", rssi);
			sleep(1);
		}
		break;
	case PRSSI_TX:
		cc1200_cmd(STX);
		sleep(1);
		while (!abort || !abort(args))
			continue;
		break;
	default:
		LERR("Invalid mode\n");
		break;
	}

	cc1200_cmd(SIDLE);
	spi_shutdown();
	return;

}