#ifndef PRSSI_H
#define PRSSI_H

#include <stdbool.h>

typedef enum {
	PRSSI_RX,
	PRSSI_TX
} prssi_mode_t;

void prssi(prssi_mode_t mode, bool(*abort)(void*), void* args);

#endif //PRSSI_H