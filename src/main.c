/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include <SPIv1.h>

#include "cc1200_reg.h"
#include "cc1200_preinit.h"
#include "cc1200_function.h"

void sigint_handler(int status) {
	printf("\nWARN: Received SIGINT: %d\n", status);
	printf("INFO: Shutting down SPI...\n");
	spi_shutdown();
	exit(1);
}

int main (void) {
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);
 
	cc1200_init_reg(RegSettings,ExtRegSettings);

	while (1) {
		cc1200_tx(50);
		cc1200_rx(cc1200_rx_preparar());
	}

	// shutdown SPI
	spi_shutdown();

	return 0;
}
