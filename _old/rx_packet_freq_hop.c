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
#define NUM_FREQ_REGS 6
#define CRC16 2

// REG_TYPE freq [NUM_FREQ][NUM_FREQ_REGS] = {
// 	{ 
// 		{ FREQ2,   0x5b },
//      		{ FREQ1,   0x80 },
//      		{ FREQ0,   0x00 },
//                 { FS_CHP,  0x2a },
//                 { FS_VCO4, 0x11 },
//                 { FS_VCO2, 0x64 }
// 	}, {
// 		{ FREQ2,   0x56 },
//      		{ FREQ1,   0xcc },
//      		{ FREQ0,   0xcc },
//                 { FS_CHP,  0x2b },
//                 { FS_VCO4, 0x11 },
//                 { FS_VCO2, 0x4c }
// 	}
// };



void sigint_handler(int status) {
	printf("\nWARN: Received SIGINT: %d\n", status);
        printf("INFO: Shutting down SPI...\n");
	spi_shutdown();
	exit(1);
}

/*
 * Returns: 0 on success; 1 on timeout
 */
int wait_till_mode(int mode, int timeout_ms, bool exit_on_timeout) {
        int curr_mode = -1;
        int wait_us = 10;
        int waited_us = 0;
        if (timeout_ms < 0)
                timeout_ms = 1000;
        while(mode != curr_mode) {
                if (timeout_ms < waited_us / 1000) {
                        if (exit_on_timeout) {
                                printf("Err: Wait timed out\n");
                                spi_shutdown();
                                exit(1);
                        }
                        return 1;
                }
                usleep(wait_us);
                waited_us += wait_us;
                cc1200_cmd(SNOP);
                curr_mode = get_status_cc1200();
        }
        return 0;
}

int wait_till_bytes_in_queue(int timeout_ms, bool exit_on_timeout) {
        int wait_us = 10;
        int waited_us = 0;
        while (!cc1200_reg_read(NUM_RXBYTES, 0)) {
                if (timeout_ms < waited_us / 1000) {
                        if (exit_on_timeout) {
                                printf("Err: Wait timed out\n");
                                spi_shutdown();
                                exit(1);
                        }
                        return 1;
                }
                usleep(wait_us);
                waited_us += wait_us;
        }
        return 0;
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
       
	// set variable length mode 
	int pkt_cfg0 = cc1200_reg_read(PKT_CFG0, 0);
        pkt_cfg0 |= 1 << 5;
        cc1200_reg_write(PKT_CFG0, pkt_cfg0);
	
	cc1200_reg_write(SYMBOL_RATE2, 0b111111);

	int counter = 0;
        int err;

        while (true) {

                cc1200_cmd(SIDLE);
		
                // 0 -> IDLE
                wait_till_mode(0, 1000, true);

		for (int i = 0; i < NUM_FREQ_REGS ;i++) {
                        int adr = freq[counter][i].adr;
                        int val = freq[counter][i].val;
		        cc1200_reg_write(adr, val);
                }

		counter = (counter+1) % NUM_FREQ;

        	// receive mode
        	cc1200_cmd(SRX);

                // 1 -> RX
                wait_till_mode(1, 1000, true);

                while (!(cc1200_reg_read(RSSI0, 0) & 1))
                        usleep(10);
                if (0 > (int) (signed char) cc1200_reg_read(RSSI1, 0))
                        continue;

                printf("WAIT BYTES\n");
                err = wait_till_bytes_in_queue(1000, false);
                //printf("WAIT end\n");
                if (err) continue;

                int len = cc1200_reg_read(RXFIFO, 0) + CRC16;
                printf("INFO: Receiving packet with length: %d\n", len);
                while (len) {
                        if (!cc1200_reg_read(NUM_RXBYTES, 0)) {
                                continue;
                        }
                        int recv = cc1200_reg_read(RXFIFO, 0);
                        if (len > 2)
                                printf("%c", (char) recv);
                        fflush(stdout);

                        len--;
                }
                printf("\nINFO: Successfully received packet\n"); 
                break;
	}

	// shutdown SPI
  	spi_shutdown();
 
 	return 0;

}

