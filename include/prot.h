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
	mutex_lock     = 0,
	mutex_unlock   = 1,
	p_exit         = 2,
	term_signal    = 3,
	tx_from_buffer = 4,
	pthread_max    = 5
} pthread_fun;

typedef bool(*func_ptr)(void* args);

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
	HANDSHAKE        = 1,
	HANDSHAKE_ACK    = 2,
	CHAT             = 3,
	CHAT_ACK         = 4,
	CHAT_NAK         = 5,
	IM_HERE          = 6,
	IM_HERE_ACK      = 7,
	CIAO             = 8,
	MESSAGE_TYPE_MAX = 9
} message_type_t;

typedef struct {
	message_type_t type;
	u_int32_t      msg_len;
	char*          msg;
	u_int32_t      crc32;
} message_t;

typedef struct {
	u_int32_t serial;
	message_t* msg;
} serial_message_t;

typedef struct {
	size_t len;
	char* str;
} mstring_t; // stands for message string

typedef struct {
	u_int32_t serial;
	mstring_t* mstr;
} serial_mstring_t;

void free_message(message_t* message);

void free_serial_message(serial_message_t* message);

void free_mstring(mstring_t* mstring);

void free_serial_mstring(serial_mstring_t* smstring);

const char* stages_str(stages_t stage);

const char* client_mode_str(client_mode_t client_mode);

void print_session(session_t* session);

inline void update_rssi_avg(session_t* session, int8_t rssi ){
	int8_t rssi_avg = session->rssi_avg;
	rssi_avg = (rssi_avg + rssi) / 2;
	session->rssi_avg  = rssi_avg;
	session->num_pkt_recv++;
}

inline void reset_sesson(session_t* session){
	session->stage = CONNECT;
	session->rssi_avg = 0;
	session->num_pkt_recv=0;
	session->num_pkt_send=0;
	session->rssi_seed = 0;
	session->client_mode=0;
}

/**
 * Checks if str is at least 5 bytes.
 * Checks if type is valid.
 * Checks if CRC32 is valid. CRC32 includes <uint8:type><char*:msg>
 * @param len  length of msg
 * @param str  <uint8:type><char*:msg><uint32:crc>
 * @return     message_t pointer on success, null on invalid crc
 */
message_t* parse_message(u_int32_t len, char* str);

/**
 * Extracts the serial from msg.
 * @param msg  message_t pointer holding a message with serial
 * @return     serial_message_t pointer
 */
serial_message_t* msg_to_serial_msg(message_t* msg);

/**
 * Checks if type is valid.
 * Generates CRC32 on <uint8:type><char*:msg>
 * @param type  type byte to prepend
 * @param body  message to parse
 * @param len   len of message
 * @return      mstring_t pointer or NULL on err
 */
mstring_t* gen_message_str(message_type_t type, char* body, size_t len);

/**
 * same like message_str plus set serial number
 * @param type    type byte to prepend
 * @param serial  serial to prepend
 * @param body    message to parse
 * @param len     len of message
 * @return        mstring_t pointer or NULL on err
 */
mstring_t* gen_serial_message_str(message_type_t type, u_int32_t serial, char* body, size_t len);

/**
 * Extracts serial from mstring_t.
 * @param mstring  mstring to extract serial from
 * @return         serial_mstring_t* on success, NULL on err
 */
serial_mstring_t* mstr_to_serial_mstr(mstring_t* mstring);

/*****************************************
 * ___  ____ ____ ___ ____ ____ ____ _    
 * |__] |__/ |  |  |  |  | |    |  | |    
 * |    |  \ |__|  |  |__| |___ |__| |___ 
 *
 *****************************************/

/**
 * Tries to handshake (refere to protocol).
 * @param session     session_t pointer for current session
 * @param abort       function pointer to a abort function (abort if returns true)
 * @param abort_args  arguments passed to function
 * @return  > 0 on error
 */
int32_t handshake(session_t* session, bool(*abort)(void*), void* abort_args);

int32_t chat(session_t* session, bool(*abort)(void*), char*(buffer)(void*), void* func_args);

#endif //PROT_H
