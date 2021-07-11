#ifndef IRCP_CONNECT_H
#define IRCP_CONNECT_H

#include <stdlib.h>

#include "ircp/utils.h"
#include "util/types.h"

int32_t handshake(session_t* session, boolfunc_t abort, void* abort_args);

#endif //IRCP_CONNECT_H