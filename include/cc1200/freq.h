#ifndef FREQ_H
#define FREQ_H

#include <stdlib.h>

#define CC1200_FXOSC 40
#define CC1200_NUM_FREQ_BANDS 6

/********************************************************************
 * CC1200 has 6 frequency bands. Those have individual LO divider.
 * As seen in the `FREQ_BANDS` array, these LO divider have a
 * binary representation in the FS_CFG.FSD_BANDSELECT register of
 * the CC1200.
 *
 * The formular to calculate from Hz to register values is:
 *     =========================================
 *     LO_div  ~ dependant on f_rf (band)
 *         1. 820 MHz   - 960 MHz -> LO_div = 4
 *         2. 410 MHz   - 480 MHz -> LO_div = 8
 *         3. 273.3 MHz - 320 MHz -> LO_div = 12
 *         4. 205 MHz   - 240 MHz -> LO_div = 16
 *         5. 164 MHz   - 192 MHz -> LO_div = 20
 *         6. 136.7 MHz - 160 MHz -> LO_div = 24
 *     fxosc   = 40 (Hz)
 *     freq    = FREQ2:FREQ1:FREQ0
 *     freqoff = FREQOFF1:FREQOFF2
 *     -----------------------------------------
 *     f_rf (MHz) = f_vco / LO_div
 *          f_vco = (freq/2^16) * fxosc (Hz)
 *                + (freqoff/2^18) * fxosc (Hz)
 *
 ********************************************************************/

typedef struct {
	u_int32_t freq_min;
	u_int32_t freq_max;
	u_int8_t lo_div;
	u_int8_t fsd_bs : 4; // fsd_bandselect
} cc1200_freq_band_t;

static const cc1200_freq_band_t FREQ_BANDS[CC1200_NUM_FREQ_BANDS] = {
	{ .freq_min = 820000, .freq_max = 960000, .lo_div = 4,  .fsd_bs = 0b0010 },
	{ .freq_min = 410000, .freq_max = 480000, .lo_div = 8,  .fsd_bs = 0b0100 },
	{ .freq_min = 273300, .freq_max = 320000, .lo_div = 12, .fsd_bs = 0b0110 },
	{ .freq_min = 205000, .freq_max = 240000, .lo_div = 16, .fsd_bs = 0b1000 },
	{ .freq_min = 164000, .freq_max = 192000, .lo_div = 20, .fsd_bs = 0b1010 },
	{ .freq_min = 136700, .freq_max = 160000, .lo_div = 24, .fsd_bs = 0b1011 }
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
 * Note: FREQOFFX is ignored because of
 *       32 bit insufficient for calc
 * Note: Precision of 1/10th of a MHz
 *       -> 915.15 > 915.1
 * @param f  frequency in KHz.
 * @return   struct of 3 FREQX bytes, NULL on unsupported freq
 */
cc1200_freq_t* cc1200_calc_freq(u_int32_t f);

/**
 * Sets FREQX and FS_CFG.FSD_BANDSELECT
 * register of CC1200.
 * @param f  frequency in KHz.
 * @return   0 on success, 1 on not IDLE, 2 on unsupported freq
 */
int32_t cc1200_set_freq(u_int32_t f);

#endif //FREQ_H