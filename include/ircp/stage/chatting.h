#ifndef IRCP_CHATTING_H
#define IRCP_CHATTING_H

#include <stdlib.h>

#include "ircp/utils.h"
#include "util/types.h"

int32_t chat(
	session_t* session,
	boolfunc_t abort,
	void* abort_args,
	strfunc_t readln,
	void* readln_args
);

#endif //IRCP_CHATTING_H