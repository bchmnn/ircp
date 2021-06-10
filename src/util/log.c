#include "util/log.h"

#if GLOBAL_LOGGING_LEVEL 
#include <stdio.h>
#include <stdarg.h>

const char* LEVEL_STR[] = {
        "SILENT", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

void _dolog(
        LOG_LEVEL LOGGING_LEVEL,
        LOG_LEVEL DOMAIN_LOGGING_LEVEL,
        const char* file,
        const char* func,
        int line,
        char* fmt,
        ...
) {
        if (LOGGING_LEVEL > DOMAIN_LOGGING_LEVEL || LOGGING_LEVEL > GLOBAL_LOGGING_LEVEL)
                return;
        va_list args;
        va_start(args, fmt);
        printf("[%s] [%s:%u:%s] ", LEVEL_STR[LOGGING_LEVEL], file, line, func);
        vprintf(fmt, args);
        va_end(args);
}
#endif