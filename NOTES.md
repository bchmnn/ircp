FS_CFG & 0b1111 =
* 0010 -> 4
* 0100 -> 8
* 0110 -> 12
* 1000 -> 16
* 1010 -> 20
* 1011 -> 24

//915 Mhz
FREQ2, 0x5b
FREQ1, 0x80
FREQ0, 0x00
FREQ = 5,996,544
foxsc = 40

//868 Mhz
FREQ2, 0x56
FREQ1, 0xcc
FREQ0, 0xcc
FREQ = 5,688,524
foxsc = 40.00000563

//433 Mhz
FREQ2, 0x2b
FREQ1, 0x4c
FREQ0, 0xcc
FREQ = 2,837,708
fxosc = 39.95382189

FREQ = FREQ2:FREQ1:FREQ0
FREQOFF = FREQOFF1:FREQOFF2

f_rf = f_vco / LO div
f_vco = (freq/2^16)*fxosc + (freqoff/2^18)*fxosc
f_vco = (freq/65,536)*40 + (freqoff/262,144)*40
