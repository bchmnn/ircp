#include "cc1200_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h> // usleep
#include <pthread.h>

#include "cc1200/utils.h"

#include "util/crypto.h"
#include "util/log.h"
#include "util/ringbuffer.h"
#include "prot.h"

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

bool get_term_signal(void* _args) {
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	bool ret;
	pthread_mutex_lock(args->term_signal_mutex);
	ret = *args->term_signal;
	pthread_mutex_unlock(args->term_signal_mutex);
	return ret;
}

bool get_pthread_mutex_lock(void* _args){
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	pthread_mutex_lock(args->term_signal_mutex);
	return 0;
}

bool get_pthread_mutex_unlock(void* _args){
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	pthread_mutex_lock(args->term_signal_mutex);
	return 0;
}

bool get_pthread_exit(){
	pthread_exit(0);
	return 0;
}

char* read_from_ringbuffer(void* _args){
	char* str = NULL;
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	pthread_mutex_lock(args->term_signal_mutex);
	if(!rb_empty(args->buf)) {
		LTRAC("Ringbuffer not empty\n");
		str = rb_pop_str(args->buf);
	}
	pthread_mutex_unlock(args->term_signal_mutex);
	return str;
}

func_ptr pthread [pthread_max]={
	get_pthread_mutex_lock ,
	get_pthread_mutex_unlock,
	get_pthread_exit,
	get_term_signal,
	//read_from_ringbuffer
};

void *cc1200_thread(void* _args) {
	LDEBG("Starting cc1200_thread\n");
	cc1200_thread_args_t* args = (cc1200_thread_args_t*) _args;
	session_t* session = malloc(sizeof(session_t));
	session->stage = CONNECT;
	
	while (!get_term_signal(args)) {
		switch (session->stage) {
		case CONNECT:
			handshake(session,(bool(*)(void*)) &get_term_signal , _args);
			print_session(session);
			break;
		case CHATTING:
			chat(session,
				(bool(*)(void*)) &get_term_signal,
				(char*(*)(void*)) &read_from_ringbuffer,
				_args
			);
			break;
		case INTERRUPTED:
			LERR("Not implemented\n");
			break;
		case RECONNECT:
			LERR("Not implemented\n");
			break;
		default:
			LERR("This should not happen\n");
			break;
		}
	}
	pthread_exit(0);
}
