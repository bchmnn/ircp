#ifndef PROT_H
#define PROT_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
	CONNECT,
	CHATTING,
	INTERRUPTED,
	RECONNECT
} stages_t;

typedef enum {
	MASTER,
	SERVANT
} client_mode_t;

typedef struct {
	stages_t       stage;
	client_mode_t  client_mode;
	size_t         num_pkt_recv;
	size_t         num_pkt_send;
	int8_t         rssi_avg;
	int8_t         rssi_seed;
	float          curr_freq;
	time_t         t_start;
} session_t;

typedef enum {
	HANDSHAKE = 1,
	HANDSHAKE_ACK = 2,
	CHAT = 3,
	CHAT_ACK = 4,
	IM_HERE = 5,
	IM_HERE_ACK = 6,
	CIAO = 7,
	MESSAGE_TYPE_MAX = 8
} message_type_t;

typedef struct {
	message_type_t type;
	u_int32_t      msg_len;
	char*          msg;
	u_int32_t      crc32;
} message_t;

typedef struct {
	size_t len;
	char* str;
} mstring_t; // stands for message string

void free_message(message_t* message);

void free_mstring(mstring_t* mstring);

const char* stages_str(stages_t stage);

const char* client_mode_str(client_mode_t client_mode);

void print_session(session_t* session);

/**
 * Checks if str is at least 5 bytes.
 * Checks if type is valid.
 * Checks if CRC32 is valid. CRC32 includes <uint8:type>\<char*:msg>
 * @param len  length of msg
 * @param str  <uint8:type><char*:msg><uint32:crc>
 * @return     message_t pointer on success, null on invalid crc
 */
message_t* parse_message(u_int32_t len, char* str);

/**
 * Checks if type is valid.
 * Generates CRC32 on <uint8:type><char*:msg>
 * @param type  type byte to prepend
 * @param body  message to parse
 * @param len   len of message
 * @return      mstring_t pointer or NULL on err
 */
mstring_t* message_str(message_type_t type, char* body, size_t len);

/**
 * Tries to handshake (refere to protocol).
 * @param session     session_t pointer for current session
 * @param abort       function pointer to a abort function (abort if returns true)
 * @param abort_args  arguments passed to function
 * @return  > 0 on error
 */
int32_t handshake(session_t* session, bool(*abort)(void*), void* abort_args);

#endif //PROT_H