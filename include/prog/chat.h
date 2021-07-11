#ifndef CHAT_H
#define CHAT_H

#include <stdbool.h>

void pchat(bool(*abort)(void*), void* args);

#endif //CHAT_H