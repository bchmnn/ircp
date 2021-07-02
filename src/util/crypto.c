#include "util/crypto.h"

#include <stdlib.h>

u_int32_t crc32(char* buffer, size_t len) {
	size_t i, j;
	u_int32_t byte, crc, mask;

	i = 0;
	crc = 0xffffffff;
	while (len ? i < len : buffer[i] != 0) {
		byte = buffer[i];
		crc = crc ^ byte;
		for (j = 0; j < 8; j++) {
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xedb88320 & mask);
		}
		i = i + 1;
	}
	return ~crc;
}
