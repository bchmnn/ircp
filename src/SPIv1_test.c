/** Test SPIv1
 *
 **/

#include <stdio.h>
#include <SPIv1.h>

#define SIDLE 0x36
#define SRX  0x34
#define SRES 0x30
int main(int argc, char *argv[])
{
    int adr, val;
    if(spi_init()){
	printf("ERROR: Initialization failed\n");
	return -1;
    }

    adr = 0x01;
    cc1200_reg_read(adr, &val);
    printf("INFO:read Adr:0x%x Val:0x%x\n", adr, val);
    printf("INFO: Status:%s\n", get_status_cc1200_str());

    adr = 0x0D;
    cc1200_reg_read(adr, &val);
    printf("INFO:read adr:0x%x val:0x%x\n", adr, val);
  
    printf("INFO: REG DUMP 0x00-0x2E\n");
    for(adr = 0x00; adr < 0x2F; adr++){
      cc1200_reg_read(adr, &val);
      printf("INFO: read adr:0x%x val:0x%x\n", adr, val);
    }
    adr = 0x04;
    val = 0xff;
    cc1200_reg_write(adr, val);
    printf("INFO:write adr:0x%x val:0x%x\n", adr, val);

    adr = 0x04;
    cc1200_reg_read(adr, &val);
    printf("INFO:read adr:0x%x val:0x%x\n", adr, val);

    adr = 0x00;
    // Status only after command was issued
    cc1200_reg_read(adr, &val);
    printf("INFO: Status:%s\n", get_status_cc1200_str());

    // Extended Register tests
    printf("INFO: extended Register reads\n");
    for(adr = 0x00; adr < 0x30; adr++){
      cc1200_reg_read(EXT_ADR | adr, &val);
      printf("INFO:read adr:0x%x val:0x%x\n", adr, val);
    }

    // Extended Register write test
    adr = EXT_ADR | 0x0A;
    cc1200_reg_read(adr, &val);
    printf("INFO:read adr:0x%x val:0x%x\n", adr, val);
    val = 0xf0;
    cc1200_reg_write(adr, val);
    cc1200_reg_read(adr, &val);
    printf("INFO:read after write 0xf0 in 0x0a adr:0x%x val:0x%x\n", adr, val);
    // Reset
    cc1200_cmd(SRES);
    spi_shutdown(); // Shut down spi.. DON'T CALL any other functions of SPIv1.h after this!



    
    return 0;
}
