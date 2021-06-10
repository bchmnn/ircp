#ifndef ASSERT_H
#define ASSERT_H

#include <config.h>

#ifdef GLOBAL_LOGGING_LEVEL

void _doassert(
        const char* file,
        int line,
        const char* func,
        int assertion,
        char* fmt,
        ...
);

#if GLOBAL_LOGGING_LEVEL >= 3
#define ASSERTF(x, fmt, ...) _doassert(__FILE__, __LINE__, __func__, x, fmt, ##__VA_ARGS__)
#define ASSERT(x) ASSERTF(x, "\n")
#else
#define ASSERTF(x, fmt, ...)
#define ASSERT(x)
#endif //GLOBAL_LOGGING_LEVEL >= 3 

#endif //GLOBAL_LOGGING_LEVEL

#endif //ASSERT_H