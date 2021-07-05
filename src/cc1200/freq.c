#include "cc1200/freq.h"

#include <stdlib.h>
#include <external/SPIv1.h>

#include "cc1200/regs.h"
#include "util/log.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

#define FS_CFG_LO_DIV_MASK  0b1111
#define FREQ_DIV            0x10000
#define FREQOFF_DIV         0x40000

u_int16_t get_freq_off() {
	return cc1200_reg_read(FREQOFF1, 0) << 8 | cc1200_reg_read(FREQOFF0, 0);
}

cc1200_freq_t* cc1200_calc_freq(u_int32_t _f) {

	int32_t band = -1;
	for (size_t i = 0; i < CC1200_NUM_FREQ_BANDS; i++) {
		if (FREQ_BANDS[i].freq_min <= _f && FREQ_BANDS[i].freq_max >= _f) {
			band = i;
			break;
		}
	}

	if (band == -1) {
		LWARN("Unsupported frequency: %u KHz\n", _f);
		return NULL;
	}

	// 9511 ~ 951.1 MHz
	u_int32_t f = _f / 100;

	cc1200_freq_t* freq = malloc(sizeof(cc1200_freq_t));

	// float f_off = (get_freq_off() * CC1200_FXOSC) / FREQOFF_DIV;
	// float f_vco = f * FREQ_BANDS[band].lo_div;
	u_int32_t _freq = ((f * FREQ_DIV) / (CC1200_FXOSC * 10)) * FREQ_BANDS[band].lo_div;
	// u_int32_t _freq = (u_int32_t) ((f_vco) / CC1200_FXOSC) * FREQ_DIV;

	freq->freq2 = _freq >> 16;
	freq->freq1 = _freq >> 8;
	freq->freq0 = _freq;
	freq->fsd_bs = FREQ_BANDS[band].fsd_bs;

	LDEBG("Calculated freq %u KHz: 0x%02x%02x%02x: LO_div %d\n",
		_f, freq->freq2, freq->freq1, freq->freq0, FREQ_BANDS[band].lo_div);

	return freq;
}

int32_t cc1200_set_freq(u_int32_t f) {

	cc1200_cmd(SNOP);
	if (get_status_cc1200() != IDLE)
		return 1;

	cc1200_freq_t* freq = cc1200_calc_freq(f);
	if (freq == NULL)
		return 2;

	u_int8_t fs_cfg = cc1200_reg_read(FS_CFG, 0);
	fs_cfg &= ~FS_CFG_LO_DIV_MASK;
	fs_cfg |= freq->fsd_bs;
	cc1200_reg_write(FS_CFG, fs_cfg);

	cc1200_reg_write(FREQ2, freq->freq2);
	cc1200_reg_write(FREQ1, freq->freq1);
	cc1200_reg_write(FREQ0, freq->freq0);

	return 0;
}
