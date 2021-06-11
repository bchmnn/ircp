#ifndef LOG_H
#define LOG_H

#include <config.h>

// defined in config.h
#ifndef GLOBAL_LOGGING_LEVEL
        #define GLOBAL_LOGGING_LEVEL 2
#endif

// set in config.h
#if GLOBAL_LOGGING_LEVEL


/**
 * Usage:
 * #include "log.h"
 * 
 * #define LOGGING_LEVEL TRACE
 * #define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
 * #define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
 * #define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
 * #define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
 * #define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
 */


typedef enum {
        SILENT  = 0,
        ERROR   = 1,
        WARNING = 2,
        INFO    = 3,
        DEBUG   = 4,
        TRACE   = 5
} LOG_LEVEL;

void _dolog(
        LOG_LEVEL LOGGING_LEVEL,
        LOG_LEVEL DOMAIN_LOGGING_LEVEL,
        const char* file,
        const char* func,
        int line,
        char* fmt,
        ...
);

#if GLOBAL_LOGGING_LEVEL >= 1
#define _LOG_ERROR(DMN_LVL, fmt, ...) _dolog(ERROR,   DMN_LVL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_ERROR(DMN_LVL, fmt, ...)
#endif
 
#if GLOBAL_LOGGING_LEVEL >= 2
#define _LOG_WARN( DMN_LVL, fmt, ...) _dolog(WARNING, DMN_LVL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_WARN( DMN_LVL, fmt, ...)
#endif

#if GLOBAL_LOGGING_LEVEL >= 3
#define _LOG_INFO( DMN_LVL, fmt, ...) _dolog(INFO,    DMN_LVL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_INFO( DMN_LVL, fmt, ...)
#endif

#if GLOBAL_LOGGING_LEVEL >= 4
#define _LOG_DEBUG(DMN_LVL, fmt, ...) _dolog(DEBUG,   DMN_LVL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_DEBUG(DMN_LVL, fmt, ...)
#endif

#if GLOBAL_LOGGING_LEVEL >= 5
#define _LOG_TRACE(DMN_LVL, fmt, ...) _dolog(TRACE,   DMN_LVL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_TRACE(DMN_LVL, fmt, ...)
#endif

#endif //GLOBAL_LOGGING_LEVEL

#endif //LOG_H