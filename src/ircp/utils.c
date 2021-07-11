#include "ircp/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/crypto.h"
#include "util/log.h"

#define LOGGING_LEVEL INFO
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void free_message(message_t* message) {
	if (!message) return;
	if (message->msg) free(message->msg);
	free(message);
}

void free_serial_message(serial_message_t* message) {
	if (!message) return;
	free_message(message->msg);
	free(message);
}

void free_mstring(mstring_t* mstring) {
	if (!mstring) return;
	if (mstring->str) free(mstring->str);
	free(mstring);
}

void free_serial_mstring(serial_mstring_t* smstring) {
	if (!smstring) return;
	if (smstring->mstr) free_mstring(smstring->mstr);
	free(smstring);
}

const char* stages_str(stages_t stage) {
	switch (stage) {
	case CONNECT:     return "CONNECT";
	case CHATTING:    return "CHATTING";
	case INTERRUPTED: return "INTERRUPTED";
	case RECONNECT:   return "RECONNECT";
	default:          return "UNKNOWN";
	}
}

const char* client_mode_str(client_mode_t client_mode) {
	switch (client_mode) {
	case MASTER:  return "MASTER";
	case SERVANT: return "SERVANT";
	default:      return "UNKNOWN";
	}
}

void print_session(session_t* session) {
	printf("session: {\n");
	printf("    stage: %s\n", stages_str(session->stage));
	printf("    client_mode: %s\n", client_mode_str(session->client_mode));
	printf("    rssi_seed: %d\n", (int32_t) session->rssi_seed);
	printf("}\n");
}

message_t* parse_message(u_int32_t len, char* str) {

	if (len < 5) {
		LDEBG("Message to short\n");
		return NULL;
	}

	if (((u_int32_t) str[0]) >= MESSAGE_TYPE_MAX) {
		LDEBG("Message has invalid type\n");
		return NULL;
	}

	u_int32_t crc = crc32(str, len - 4);
	u_int32_t crc_given = *((u_int32_t*) (str + len - 4));

	if (crc != crc_given) {
		LDEBG("CRC was invalid: given: 0x%08x calc: 0x%08x\n", crc_given, crc);
		return NULL;
	}

	message_t* msg = malloc(sizeof(message_t));
	if (len > 5) {
		msg->msg = malloc(sizeof(char) * ((len - 5) + 1));
		memcpy(msg->msg, (str + 1), len - 5);
		msg->msg[len - 5] ='\0';
	} else {
		msg->msg = NULL;
	}

	msg->msg_len = len - 5;
	msg->type = (u_int32_t) str[0];
	msg->crc32 = crc;

	return msg;

}

serial_message_t* msg_to_serial_msg(message_t* msg) {
	if (!msg || !msg->msg)
		return NULL;
	if (msg->msg_len < 4) {
		LWARN("parse_serial_message requires at least 4 bytes\n");
		return NULL;
	}

	serial_message_t* smsg = malloc(sizeof(serial_message_t));
	smsg->serial = *((u_int32_t*) msg->msg);
	if (msg->msg_len > 4) {
		char* str = malloc(sizeof(char) * (msg->msg_len - 4 + 1));
		memcpy(str, (msg->msg + 4), msg->msg_len - 4);
		str[msg->msg_len - 4] = '\0';
		free(msg->msg);
		msg->msg = str;
		msg->msg_len = msg->msg_len - 4;
	} else {
		msg->msg = NULL;
		msg->msg_len = 0;
		free(msg->msg);
	}
	smsg->msg = msg;
	return smsg;
}


mstring_t* gen_message_str(message_type_t type, char* body, size_t len) {

	if (type >= MESSAGE_TYPE_MAX) {
		LDEBG("Invalid type\n");
		return NULL;
	}

	if (body == NULL && len > 0) {
		LDEBG("Lenght can't be greater zero when body is NULL\n");
		return NULL;
	}

	mstring_t* mstring = malloc(sizeof(mstring_t));
	mstring->str = malloc(sizeof(char) * (len + 6));

	mstring->str[0] = (u_int8_t) type;

	if (len) memcpy((mstring->str + 1), body, len);

	u_int32_t crc = crc32(mstring->str, len + 1);
	memcpy((mstring->str + len + 1), (char*) &crc, 4);

	mstring->str[len + 5] = '\0';
	mstring->len = len + 5;

	return mstring;
}

mstring_t* gen_serial_message_str(message_type_t type, u_int32_t serial, char* body, size_t len) {

	char* smessge_str = malloc(sizeof(char) * (len + 4));
	*((u_int32_t*) smessge_str) = serial;
	memcpy((smessge_str + 4), body, len);

	return gen_message_str(type, smessge_str, len + 4);

}

serial_mstring_t* mstr_to_serial_mstr(mstring_t* mstring) {

	if (!mstring || !mstring->str)
		return NULL;

	// <type:1><serial:4><crc32:4>
	if (mstring->len < 9) {
		LWARN("Len must be at least 9 to parse serial: <type:1><serial:4><crc32:4>\n");
		return NULL;
	}

	u_int32_t serial = *((u_int32_t*) (mstring->str + 1));
	serial_mstring_t* smstr = malloc(sizeof(serial_mstring_t));

	smstr->mstr = mstring;
	smstr->serial = serial;

	return smstr;

}

