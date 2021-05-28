/usr/bin/cc -I/home/debian/test/include -std=gnu99 -o main.c.o -c /home/debian/test/main.c
/usr/bin/cc -L/usr/local/lib -lspi -Wl,-rpath=/usr/local/lib -L/usr/local/lib -lprussdrv main.c.o -o target -rdynamic

arm-linux-gnueabihf-gcc.exe -O3 -mcpu=cortex-a8 -Wall -Wextra "-Wl,-rpath=lib" -Iinclude -Llib -lprussdrv -lspi -o target main.c

## Execute local script/binary on target machine
### Windows
* requires visual build tools (for cmake and nmake) https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019
* pass env variables via -D ..=.. to cmake

## Remove SSH banner
* `sudo vim /etc/ssh/sshd_config`: comment out the banner line
* `sudo service sshd reload`

----------------------------

Actors:
  1 - sender/receiver
  2 - sender/receiver
  3 - interrupter

Stages:
  1 - Handshake
  2 - Chatting
  3 - Interrupted
  4 - After switch

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
    - if problem persists reduce further or switch to other frequency
  - 1/2 meassure high RSSI
    - possibility of 3 - interrupter
    - switch frequency acording to SEED
4. After switch
  - 1 loops: send "i'm here", listen for ACK
    -> 1 goes into chatting listen mode
  - 2 loop: listen "i'm here" -> on receive: send ACK
    -> 2 goes into chatting listen mode
    -> if recv "i'm here" -> resend ACK
