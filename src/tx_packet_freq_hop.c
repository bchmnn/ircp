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
#include "cc1200_mode_tx_packet.h"

#define MS_IN_U(ms) (ms * 1000)

#define NUM_PREAMBLE_30_B 0b110100

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

        int pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
        pkt_cfg0 |= 1 << 5;
        cc1200_reg_write(PKT_CFG0, pkt_cfg0);

        cc1200_reg_write(PREAMBLE_CFG1, NUM_PREAMBLE_30_B);

        // sym_rate2 = cc1200_reg_read(SYMBOL_RATE2, 0);
        int sym_rate2 = 0b111111;
        cc1200_reg_write(SYMBOL_RATE2, sym_rate2);

 	// get status information
        // SNOP command has to be executed prior to status retrieval
  	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());

        cc1200_reg_write(PKT_LEN, 0xff);

	int len = cc1200_reg_read(PKT_LEN, 0);
	printf("INFO: Configured packet len: %d\n", len);

	char *data ="HalloHallo\0";
	cc1200_reg_write(TXFIFO, 10);

	while (*data != '\0') {
		int c = *data;
		cc1200_reg_write(TXFIFO , c);
		printf("DEBUG: int: %d , char:  %c \n", c, c);
		usleep(MS_IN_U(10));
		data++;
	}

	usleep(MS_IN_U(100));

	int fifo = cc1200_reg_read(NUM_TXBYTES, 0);
	printf("INFO: Number of bytes in tx fifo: %d\n", fifo);
        printf("INFO: Putting into transmit mode...\n");
        // transmit  mode
	cc1200_cmd(STX);

	for (int i = 0; i < 100000000; i++) {}

	// usleep(MS_IN_U(1));
	cc1200_cmd(SNOP);
  	printf("INFO: Status: %s\n", get_status_cc1200_str());
 
	usleep(MS_IN_U(100));
	fifo = cc1200_reg_read(NUM_TXBYTES, 0);
	printf("INFO: Number of bytes in tx fifo: %d\n", fifo);               
	
	while (1) {}

	// shutdown SPI
  	spi_shutdown();
 
 	return 0;

}

