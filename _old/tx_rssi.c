/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/
 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <SPIv1.h>
#include "cc1200_reg.h"
#include "cc1200_mode_tx_rssi.h"

void sigint_handler(int status) {
	printf("\nWARN: Received SIGINT: %d\n", status);
        printf("INFO: cc1200 in iDLE mode...\n");
	cc1200_cmd(SIDLE);
	printf("INFO: Shutting down SPI...\n");
	spi_shutdown();
	exit(1);
}
 
int main (void) {

  	// first initialize
        printf("INFO: Initializing SPI\n");
  	if (spi_init()) {
    		printf("ERR: Initialization failed\n");
    		return -1;
	}

	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);

	
	// reset CC1200
	// cc1200 is now in idle mode, registers have their default values
        cc1200_cmd(SRES);

  	// reprogram the registers
        cc1200_regs_write(RegSettings, MAX_REG);
        cc1200_regs_write(ExtRegSettings, MAX_EXT_REG);

 	// get status information
        // SNOP command has to be executed prior to status retrieval
  	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());

        printf("INFO: Putting into receive mode...\n");
        // Transmit mode
	cc1200_cmd(STX);
        sleep(1);
	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());
	//signed char rssi = 0;
	while (true) {
		sleep(1);

	}

	// shutdown SPI
  	spi_shutdown();
 
 	return 0;

}

