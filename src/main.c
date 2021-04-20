
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include "smartrf_CC1200.h" // import register settings
#include "smartrf_adr_CC1200.h" // import register addresses
 
int main (void) {
 
 int adr;
 int val;
 
  // first initialize
  if(spi_init()){
    printf("ERROR: Initialization failed\n");
    return -1;
 
  // do some register reading or writing, 
  // performance commands and get status information
 
  // reset CC1200
  cc1200_cmd(SRES);
 
  // CC1200 is now in idle mode, registers have their default values
  // Reprogram the registers
  cc1200_reg_write(IOCFG2_ADR, SMARTRF_SETTING_IOCFG2);
  cc1200_reg_write(IOCFG0_ADR, SMARTRF_SETTING_IOCFG0);
 
  // ... reprogram the remaining registers 
 
  // Programm the RF frequency
  cc1200_reg_write(FREQ2_ADR, SMARTRF_SETTING_FREQ2);
  cc1200_reg_write(FREQ1_ADR, SMARTRF_SETTING_FREQ1);
  cc1200_reg_write(FREQ0_ADR, SMARTRF_SETTING_FREQ0);
 
  // ... reprogram the remaining registers
 
  cc1200_reg_write(SERIAL_STATUS_ADR, SMARTRF_SETTING_SERIAL_STATUS);  
 
 
  // get status information
  cc1200_cmd(SNOP);
  printf("INFO: Status:%s\n", get_status_cc1200_str());
 
  adr = 0x01;
  // register read
  cc1200_reg_read(adr, &val);
  printf("INFO:read Adr:0x%x Val:0x%x\n", adr, val);
 
  // read extended register
  adr = EXT_ADR | 0x0A;
  cc1200_reg_read(adr, &val);
 
  // shutdown SPI
  spi_shutdown();
 
  return 0;
}
