#ifndef CC1200_THREAD_H
#define CC1200_THREAD_H

#include <pthread.h>

#include "util/ringbuffer.h"

typedef struct {
	rb_t* buf;
	pthread_mutex_t* buf_mutex;
	bool* term_signal;
	pthread_mutex_t* term_signal_mutex;
} cc1200_thread_args_t;

void set_term_signal(cc1200_thread_args_t* args);

void *cc1200_thread(void* _args);

#endif //CC1200_THREAD_H