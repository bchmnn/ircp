#include "ircp/stage/interrupted.h"

#include <external/SPIv1.h>

#include "cc1200/freq.h"
#include "cc1200/utils.h"
#include "ircp/utils.h"
#include "util/log.h"
#include "config.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void handle_interrupt(session_t* session) {
	if (!session)
		return;

	cc1200_cmd(SIDLE);
	if (cc1200_wait_till_mode(IDLE, 1000)) {
		LERR("Could not switch into IDLE mode\n");
		return;
	}

	u_int32_t curr_freq = session->curr_freq / 1000; // MHz
	u_int32_t freq_delta = FREQ_MAX - FREQ_MIN;
	u_int32_t freq_step  = session->rssi_seed == 0 ? 1 : (u_int32_t) session->rssi_seed;
	u_int32_t next_freq = curr_freq + freq_step;
	if (next_freq > FREQ_MAX)
		next_freq = ((next_freq - FREQ_MAX) % freq_delta) + FREQ_MIN;

	next_freq *= 1000; // KHz
	if (cc1200_set_freq(next_freq)) {
		LERR("Frequency switch failed\n");
		return;
	}

	session->curr_freq = next_freq;
	session->stage = CHATTING;
	return;

}
