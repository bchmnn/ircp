#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef void*(*threadfunc_t)(void*);
typedef void(*routine_t)(void*);
typedef bool(*boolfunc_t)(void*);
typedef char*(*strfunc_t)(void*);

#endif //TYPES_H