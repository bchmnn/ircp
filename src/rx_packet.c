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
#include "cc1200_mode_rx_packet.h"

#define CRC16 2

void sigint_handler(int status) {
	printf("\nWARN: Received SIGINT: %d\n", status);
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
        int pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
        pkt_cfg0 |= 1 << 5;
        cc1200_reg_write(PKT_CFG0, pkt_cfg0);

 	// get status information
        // SNOP command has to be executed prior to status retrieval
  	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());

        printf("INFO: Putting into receive mode...\n");
        // receive mode
	cc1200_cmd(SRX);
        sleep(1);

	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());
        int len = cc1200_reg_read(PKT_LEN, 0);
        // int buff_p = 0;
        // int *buff = malloc(sizeof(int)*(len+2));

	int data;
	while (true) {
		int variable = cc1200_reg_read(NUM_RXBYTES,0);
                if (!variable)
                        continue;

                len = cc1200_reg_read(RXFIFO, 0) + CRC16;
                printf("INFO: Receiving packet with length: %d\n", len);
                while (len) {
                        if (!cc1200_reg_read(NUM_RXBYTES, 0)) {
                                continue;
                        }
                        int recv = cc1200_reg_read(RXFIFO, 0);
                        // printf("%c:%d|", (char) recv, (int) recv);
                        if (len > 2)
                                printf("%c", (char) recv);
                        fflush(stdout);

                        len--;
                }
                printf("\nINFO: Successfully received packet\n");
                
	}

	// shutdown SPI
  	spi_shutdown();
 
 	return 0;

}

