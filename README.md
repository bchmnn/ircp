# Network Protocol Programming Lab (PPL)

## Hardware
* Beagleboard Black
* CC1200

## Links
* [ Begleboard getting-started](https://beagleboard.org/getting-started/)
* [Begleboard Linux] (https://beagleboard.org/linux/)
* [TKN Wiki ](https://kn-pr.tkn.tu-berlin.de/wiki/doku.php)
*  [User’s Guide of the CC1200](http://www.ti.com/lit/ug/swru346b/swru346b.pdf)



## TODOs
* [x] Setup Beagleboards
* [x] Expose SSH
* [ ] Setup deploy-script
* [ ] Setup CI/CD



## Basic inforamtion

 basic registers  0x00
 extended registers. 0x2F (0x2F00 offset) 


int spi_init(void)  		wenn succes the func return true

void spi_shutdown(void)        corresponding to spi_init

int cc1200_reg_write(int adr, int val) return the written value.

int cc1200_reg_read(int adr, int *val) return the register value


int cc1200_cmd(int cmd)

int get_status_cc1200(void) returns the last available status

char *get_status_cc1200_str(void) returns the status information as a string.




