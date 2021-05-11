# Network Protocol Programming Lab (PPL)

## Hardware
* Beagleboard Black
* CC1200

RSSI: SNR - Signal to noice ratio
      SIR - Signal to interference ratio
      Symbolrate - Symbols per second
      Bitrate - Bits per second

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

## Building
* Use the bash-script `fast` for that
```
$ ./fast help
Usage: ./fast <command>
Commands:
    cmake   runs cmake in dir target
    build   runs cmake --build target
    exec    runs the target binary
    clean   rm -rf ./target
    help    show this screen
```
* The resulting executable will be ./target/main

## Basic inforamtion
* CC1200: basic registers: 0x00
* CC1200: extended registers: 0x2fxx (0x2f00 offset) 

```c
// returns true on success
int spi_init(void);

// has to be called if `spi_init` was called before
void spi_shutdown(void);

// returns written value
int cc1200_reg_write(int adr, int val);

// returns read value
int cc1200_reg_read(int adr, int *val);

int cc1200_cmd(int cmd);

// cc1200_cmd(SNOP) has to be called before
// returns last available status
int get_status_cc1200(void);

// return last available status as string
char *get_status_cc1200_str(void);
```


