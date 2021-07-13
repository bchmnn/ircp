#ifndef PRSSI_H
#define PRSSI_H

#include <stdlib.h>
#include <stdbool.h>
#include "util/types.h"

typedef enum {
	PRSSI_RX,
	PRSSI_TX
} prssi_mode_t;

void prssi(prssi_mode_t mode, u_int32_t freq, boolfunc_t abort, void* args);

#endif //PRSSI_H