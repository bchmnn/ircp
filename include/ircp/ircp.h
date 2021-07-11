#ifndef IRCP_H
#define IRCP_H

#include "util/types.h"

typedef struct {
	boolfunc_t abort;
	void*      abort_args;
	strfunc_t  readln;
	void*      readln_args;
} ircp_exec_args_t;

void ircp_exec(ircp_exec_args_t* args);

#endif //IRCP_H