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
* Download toolchain from: https://releases.linaro.org/components/toolchain/binaries/
* Select a version (`latest-7` worked for me)
* Download, extract, copy to desired location, add to path
* Let vscode generate `c_cpp_properties.json` for you and then change `"configurations"."compilerPath"` to the `gcc` version of the linaro toolchain

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

## Vocabulary
* **RSSI/SNR** - Signal to noice ratio
* **SIR** - Signal to interference ratio
* **Symbolrate** - Symbols per second
* **Bitrate** - Bits per second
