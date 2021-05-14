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

** status register 


/* command strobes */
+ SRES     0x30 /**< CC1200 in den Ausgangszustand setzen (Chip reset). */
+ SFSTXON  0x31 /**< Schalte den Frequenzsynthesizer ein und kalibriere ihn. */
+ SXOFF    0x32 /**< Gehe in den XOFF Zustand. */
+ SCAL     0x33 /**< Kalibriere Frequenzsynthesizer und schalte ihn aus. */
+ SRX      0x34 /**< Kalibriere Chip und schalte in den Empfangsmodus. */
+ STX      0x35 /**< Schalte in den Sendemodus. */
+ SIDLE    0x36 /**< Gehe in den Ruhezustand. */
+ SAFC     0x37 /**< Führe eine automatische Frequenz-Kompensation (AFC) aus. */
+ SWOR     0x38 /**< Starte die automatische RX polling Sequenz. */
+ SPWD     0x39 /**< Gehe in des SLEEP Mode. */
+ SFRX     0x3A /**< Lösche den RX FIFO. */
+ SFTX     0x3B /**< Lösche den TX FIFO. */
+ SWORRST  0x3C /**< Setze die eWOR Zeit. */
+ SNOP     0x3D  /**< Kein Kommando. 
                           * Wird benutzt um den Zustand des CC1200 zu ermitteln */


