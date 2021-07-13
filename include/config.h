#ifndef CONFIG_H
#define CONFIG_H

// 0:silent 1:error 2:warning 3:info 4:debug 5:trace
#define GLOBAL_LOGGING_LEVEL 5

// CC1200
#define FREQ_MIN 820
#define FREQ_MAX 950

#define RSSI_TOLERANCE 10

// Program chat
#define STDIN_BUF_SIZE 2048
#define SERIAL_TX_BUF_SIZE 512
#define SERIAL_RX_BUF_SIZE 512
#define RESEND_AFTER_ITERATIONS 10


#endif //CONFIG_H