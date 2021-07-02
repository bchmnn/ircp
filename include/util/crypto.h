#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdlib.h>

/**
 * @param len  if len = 0 then buffer is iterated till null terminator
 */
u_int32_t crc32(char* buffer, size_t len);

#endif //CRYPTO_H