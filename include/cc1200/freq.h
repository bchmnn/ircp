#ifndef FREQ_H
#define FREQ_H

#include <stdlib.h>

#define CC1200_FXOSC 40
#define CC1200_NUM_FREQ_BANDS 6

typedef struct {
	float freq_min;
	float freq_max;
	u_int8_t lo_div;
	u_int8_t fsd_bs : 4; // fsd_bandselect
} cc1200_freq_band_t;

static const cc1200_freq_band_t FREQ_BANDS[CC1200_NUM_FREQ_BANDS] = {
	{ .freq_min = 820,   .freq_max = 960, .lo_div = 4,  .fsd_bs = 0b0010 },
	{ .freq_min = 410,   .freq_max = 480, .lo_div = 8,  .fsd_bs = 0b0100 },
	{ .freq_min = 273.3, .freq_max = 320, .lo_div = 12, .fsd_bs = 0b0110 },
	{ .freq_min = 205,   .freq_max = 240, .lo_div = 16, .fsd_bs = 0b1000 },
	{ .freq_min = 164,   .freq_max = 192, .lo_div = 20, .fsd_bs = 0b1010 },
	{ .freq_min = 136.7, .freq_max = 160, .lo_div = 24, .fsd_bs = 0b1011 }
};

typedef struct {
	u_int8_t freq2;
	u_int8_t freq1;
	u_int8_t freq0;
	u_int8_t fsd_bs : 4; // fsd_bandselect
} cc1200_freq_t;

/**
 * Calculates 3 FREQX bytes from MHz.
 * Reads register FS_CFG for LO divider
 * and FREQOFFX for frequency offset.
 * @param f  frequency in MHz.
 * @return   struct of 3 FREQX bytes, NULL on unsupported freq
 */
cc1200_freq_t* cc1200_calc_freq(float f);

/**
 * Sets FREQX and FS_CFG.FSD_BANDSELECT
 * register of CC1200.
 * @param f  frequency in MHz.
 * @return   0 on success, 1 on not IDLE, 2 on unsupported freq
 */
int32_t cc1200_set_freq(float f);

#endif //FREQ_H