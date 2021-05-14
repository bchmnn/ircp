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


#define NUM_FREQ 2
#define NUM_FREQ_REGS 3
#define CRC16 2

REG_TYPE freq [NUM_FREQ][NUM_FREQ_REGS] = {
	{ 
		{FREQ2           , 0x5b },  // Frequency Configuration [23:16]
     		{FREQ1           , 0x80 },  // Frequency Configuration [15:8]
     		{FREQ0           , 0x00 }  // Frequency Configuration [7:0]
	},
	{
		{FREQ2           , 0x56 },  // Frequency Configuration [23:16]
     		{FREQ1           , 0xcc },  // Frequency Configuration [15:8]
     		{FREQ0           , 0xcc }  // Frequency Configuration [7:0]
	}
};



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
       
	//set  variable length mode 
	int pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
        pkt_cfg0 |= 1 << 5;
        cc1200_reg_write(PKT_CFG0, pkt_cfg0);
	

	

	cc1200_reg_write(SYMBOL_RATE2, 0b111111);

	int counter = 0;
        while(true){
		
		cc1200_cmd(SIDLE);
		usleep(1000);
		// get status information
        	// SNOP command has to be executed prior to status retrieval
        	cc1200_cmd(SNOP);
        	printf("INFO: Status: %s\n", get_status_cc1200_str());
		
		for (int i = 0; i < NUM_FREQ_REGS ;i++){
			 cc1200_reg_write(freq[counter][i].adr,freq[counter][i].val);       

		}
		counter = (counter+1)%NUM_FREQ;
		//printf("INFO: Putting into receive mode...\n");
        	// receive mode
        	cc1200_cmd(SRX);
        	usleep(1000);	
		//check is rssi valid
		if( !(cc1200_reg_read(RSSI0,0) & 1)){
		 	continue;
			printf("VAILD \n");
		}
		int variable = cc1200_reg_read(NUM_RXBYTES,0);
                usleep(1000);
		//	if (!variable)
                //        continue;

                int len = cc1200_reg_read(RXFIFO, 0) + CRC16;
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

