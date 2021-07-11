#include "ircp/ircp.h"

#include <stdlib.h>

#include "cc1200/utils.h"
#include "ircp/stage/chatting.h"
#include "ircp/stage/connect.h"
#include "ircp/stage/interrupted.h"
#include "ircp/stage/reconnect.h"
#include "ircp/utils.h"
#include "util/log.h"
#include "util/types.h"
#include "config.h"

#define LOGGING_LEVEL DEBUG
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void ircp_exec(ircp_exec_args_t* args) {

	LDEBG("Executing IRCP\n");
	static session_t session = { .stage = CONNECT };
	
	while (!args->abort(args->abort_args)) {
		switch (session.stage) {
		case CONNECT:
			handshake(&session, args->abort, args->abort_args);
			print_session(&session);
			break;
		case CHATTING:
			chat(
				&session,
				args->abort,
				args->abort_args,
				args->readln,
				args->readln_args
			);
			break;
		case INTERRUPTED:
			LERR("Not implemented\n");
			break;
		case RECONNECT:
			LERR("Not implemented\n");
			break;
		default:
			LERR("This should not happen\n");
			break;
		}
	}
}
