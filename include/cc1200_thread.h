#ifndef CC1200_THREAD_H
#define CC1200_THREAD_H

#include <pthread.h>

#include "util/ringbuffer.h"

/**
 * Mutexes have to be initializes statically
 */
typedef struct {
	rb_t* buf;
	pthread_mutex_t* buf_mutex;
	bool* term_signal;
	pthread_mutex_t* term_signal_mutex;
} cc1200_thread_args_t;

/**
 * Sets cc1200_thread_args_t::term_signal savely
 * to true.
 * @param args  cc1200_thread_args_t pointer
 */
void set_term_signal(cc1200_thread_args_t* args);

void *cc1200_thread(void* _args);

#endif //CC1200_THREAD_H