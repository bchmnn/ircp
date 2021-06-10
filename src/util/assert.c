#include "util/assert.h"

#include <stdio.h>
#include <stdarg.h>

void _doassert(
        const char* file,
        int line,
        const char* func,
        int assertion,
        char* fmt,
        ...
) {
        if (assertion) return;
        va_list args;
        va_start(args, fmt);
        printf("[ASSERT FAILED] [%s:%u:%s] ", file, line, func);
        vprintf(fmt, args);
        va_end(args);
}