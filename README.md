# Network Protocol Programming Lab (PPL)

## Hardware
* Beagleboard Black
* CC1200

## Links
* [Begleboard getting-started](https://beagleboard.org/getting-started/)
* [Begleboard Linux](https://beagleboard.org/linux/)
* [TKN Wiki](https://kn-pr.tkn.tu-berlin.de/wiki/doku.php)
* [User’s Guide of the CC1200](http://www.ti.com/lit/ug/swru346b/swru346b.pdf)
* [Overleaf](https://www.overleaf.com/project/60a3f53ab54bfe34a84f8c81)

## Systemd services (cc1200d and 4C)
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
* Use the script `fake` for that: https://github.com/bchmnn/fake
* The resulting executable will be `./target/target`

## Building - Cross compiling
### Setup - Toolchain
* Install `ninja`
* Download toolchain from: https://releases.linaro.org/components/toolchain/binaries/
* Select a version (`latest-7` worked for me)
* Download, extract, copy to desired location, add to path
* Let vscode generate `c_cpp_properties.json` for you and then change `"configurations"."compilerPath"` to the `gcc` version of the linaro toolchain
* Also change `cStandard` in `c_cpp_properties.json` to `gnu99`

### Setup - Fake
* Create a file in the repositories root folder:
    * Windows: `fake.conf.ps1`
    * Unix: `fake.conf.sh`
* This file will be sourced by the `fake` script
* Use this file to set some environment variables (ps1):
    ```ps1
    # ps1 version (unix analog)
    $env:CC=(which arm-linux-gnueabihf-gcc)
    $FAKE_CMAKE_FLAGS='-D CMAKE_SYSTEM_NAME=Generic -G "Ninja"'
    $FAKE_EXEC_HOST="beagle1"
    ```
* If the SSH banner is annoying you:
    * `sudo vim /etc/ssh/sshd_config`: comment out the banner line
    * `sudo service sshd reload`

## Protocol
```
=================Definitions=================
Actors:
  1 - sender/receiver
  2 - sender/receiver
  3 - interrupter
---------------------------------------------
Stages:
  1 - Handshake
  2 - Chatting
  3 - Interrupted
  4 - After switch
---------------------------------------------
Message:
  - max size: 255 bytes
  - format: <len><type:uint8><msg><checksum>
  - types:
    - 1: handshake
    - 2: handshake-ack: <msg> = rssi
    - 3: chat: <msg> = text
    - 4: chat-ack
    - 5: i'm here
    - 6: i'm here-ack
==================Protocol===================
1. Handshake
  - predefined meeting frequency: 920 hz
  - 1 and 2 handshake:
    - 2 answers with RSSI (Ack)
    - RSSI in Ack is used as SEED
    - (opt: keep key exchange in ming for crypto)
2. Chatting
  - tbd
3. Interrupted
  - 1/2 meassure high CRC and low RSSI
    - channel is corrupt
    - reduce symbol rate
    - if problem persists reduce further or
      switch to other frequency
  - 1/2 meassure high RSSI
    - possibility of 3 - interrupter
    - switch frequency acording to SEED
4. After switch
  - 1 loops: send "i'm here", listen for ACK
    -> 1 goes into chatting listen mode
  - 2 loop: listen "i'm here" -> on receive: send ACK
    -> 2 goes into chatting listen mode
    -> if recv "i'm here" -> resend ACK
```
### Notes
* google for: packet discrimination / ursache paket fehler

### Links:
* [Wiki: Interference](https://en.wikipedia.org/wiki/Interference_(communication))
* [Satcom RFI Recognition&Classification](https://www.mdpi.com/2076-3417/10/13/4608)
* [RFI Classification](https://academic.oup.com/mnras/article/405/1/155/1020990?login=true)

## Vocabulary
* **RSSI/SNR** - Signal to noice ratio
* **SIR** - Signal to interference ratio
* **Symbolrate** - Symbols per second
* **Bitrate** - Bits per second
