#include "cc1200_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h> // usleep
#include <pthread.h>

#include "util/ringbuffer.h"
#include "cc1200_function.h"
#include "util/log.h"

#define LOGGING_LEVEL DEBUG
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void set_term_signal(cc1200_thread_args_t* args) {
	pthread_mutex_lock(args->term_signal_mutex);
	*args->term_signal = true;
	pthread_mutex_unlock(args->term_signal_mutex);
}

bool get_term_signal(cc1200_thread_args_t* args) {
	bool ret;
	pthread_mutex_lock(args->term_signal_mutex);
	ret = *args->term_signal;
	pthread_mutex_unlock(args->term_signal_mutex);
	return ret;
}

void *cc1200_thread(void* _args) {
	LDEBG("Starting cc1200_thread\n");
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	while (!get_term_signal(args)) {
		pthread_mutex_lock(args->buf_mutex);
		while (!rb_empty(args->buf)) {
			LTRAC("Ringbuffer not empty\n");
			char* str = rb_pop_str(args->buf);
			size_t len = strlen(str);
			cc1200_tx(str, len);
			free(str);
		}
		pthread_mutex_unlock(args->buf_mutex);
		LTRAC("CC1200: Receiving\n");
		cc1200_pkt_t* pkt = cc1200_rx(cc1200_rx_preparar());
		if (pkt && pkt->len > 0)
			printf("%s\n", pkt->pkt);
		if (pkt)
			free_cc1200_pkt(pkt);
		usleep(10);
	}
	LDEBG("cc1200_thread received term_signal\n");
	pthread_exit(0);
}
