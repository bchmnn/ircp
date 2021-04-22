/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/
 

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available

#include "smartrf_CC1200.h" // import register settings
// #include "smartrf_adr_CC1200.h" // import register addresses

void sigint_handler(int status) {
	printf("\nReceived SIGINT:%d\n", status);
	spi_shutdown();
	exit(1);
}
 
int main (void) {
	int adr;
	int val;

  	// first initialize
  	if (spi_init()) {
    		printf("ERROR: Initialization failed\n");
    		return -1;
	}

	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);

	
	// reset CC1200
        cc1200_cmd(SRES);

	// CC1200 is now in idle mode, registers have their default values
  	// Reprogram the registers
	for (int i = 0; i < MAX_REG; i++) {
  		cc1200_reg_write(RegSettings[i].adr,RegSettings[i].val);
  	}
	
	// Programm the RF frequency
  	for (int i=0; i<MAX_EXT_REG; i++) {
		cc1200_reg_write(ExtRegSettings[i].adr, ExtRegSettings[i].val);
	}

 	// get status information
  	cc1200_cmd(SNOP);
  	printf("INFO: Status:%s\n", get_status_cc1200_str());

	// adr = 0x01;
  	// register read
  	// cc1200_reg_read(adr, &val);
  	// printf("INFO:read Adr:0x%x Val:0x%x\n", adr, val);

	// read extended register
  	// adr = EXT_ADR | 0x0A;
  	// cc1200_reg_read(adr, &val);

	cc1200_cmd(SRX);
	cc1200_cmd(SNOP);
  	printf("INFO: Status:%s\n", get_status_cc1200_str());
	while (1) {
		cc1200_reg_read(RSSI1, &val);
		printf("RSSI1: %d\n", val << 3);
		sleep(1);
	}

	// shutdown SPI
  	spi_shutdown();
 
 	return 0;

}

