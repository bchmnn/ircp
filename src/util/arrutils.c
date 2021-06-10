#include "util/arrutils.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

size_t _arrlen(void* _a, size_t type) {

	u_int8_t* a = (u_int8_t*) _a;
	size_t size = 0;
	bool is_null;

	while(true) {
		is_null = true;
		for (size_t i = 0; i < type; i++) {
			if (*a++ != 0) {
				is_null = false;
				break;
			}
		}
		if (is_null) break;
		size++;
	}

	return size;
}
