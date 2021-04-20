# Network Protocol Programming Lab (PPL)

## Hardware
* Beagleboard Black
* CC1200

## Links
* [Begleboard getting-started](https://beagleboard.org/getting-started/)
* [Begleboard Linux](https://beagleboard.org/linux/)
* [TKN Wiki](https://kn-pr.tkn.tu-berlin.de/wiki/doku.php)
* [User’s Guide of the CC1200](http://www.ti.com/lit/ug/swru346b/swru346b.pdf)

## TODOs
* [x] Setup Beagleboards
* [x] Expose SSH
* [ ] Setup deploy-script
* [ ] Setup CI/CD

## Services
* Copy the `./docs/*.service` files to `/etc/systemd/system/.`
* Extract `./archives/*` to `/srv/.`
	* `/srv/cc1200d/cc1200d` needs to be executable
	* `/usr/bin/node /srv/CC1200-Control-Center/4C.js` needs to be executable
	* in `/srv/CC1200-Control-Center/4C.js` change BeagleBone to `127.0.0.1`
	* in `/srv/CC1200-Control-Center/4C.js` at the end change the port of the webapp accordingly
* `sudo systemctl daemon-reload`
* `sudo systemctl enable <name>.service`
* `sudo systemctl start <name>.service` or `sudo reboot`

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




