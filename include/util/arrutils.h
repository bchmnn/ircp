#ifndef ARRUTILS_H
#define ARRUTILS_H

#include <stddef.h>

#define arrlen(_a, t) _arrlen(_a, sizeof(t))
size_t _arrlen(void* _a, size_t type);

#endif //ARRUTILS_H