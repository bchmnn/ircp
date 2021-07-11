#include "prog/chat.h"

#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <external/SPIv1.h>

#include "cc1200/config.h"
#include "cc1200/control.h"
#include "cc1200/freq.h"
#include "cc1200/utils.h"
#include "ircp/ircp.h"
#include "util/log.h"
#include "util/ringbuffer.h"
#include "util/readline.h"
#include "util/types.h"
#include "config.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

static rb_t*           STDIN_BUFFER = NULL;
static pthread_mutex_t STDIN_BUFFER_MUTEX;

char* readln() {
	char* str = NULL;
	pthread_mutex_lock(&STDIN_BUFFER_MUTEX);
	if (STDIN_BUFFER && !rb_empty(STDIN_BUFFER)) {
		str = rb_pop_str(STDIN_BUFFER);
	}
	pthread_mutex_unlock(&STDIN_BUFFER_MUTEX);
	return str;
}

void pchat(boolfunc_t abort, void* args) {

	if (spi_init()) {
		LERR("Initialization failed\n");
		return;
	}

	cc1200_cmd(SRES);
	if (cc1200_wait_till_mode(IDLE, 1000)) {
		LERR("Reset timed out\n");
		return;
	}

	cc1200_regs_write(CC1200_BASE_CONFIG, CC1200_NUM_REGS);
	cc1200_set_packet_mode(VAR_LENGTH, 127);

	if (cc1200_set_freq(915100)) {
		LERR("Could not set frequency\n");
		return;
	}

	// initialize thread
	STDIN_BUFFER = rb_init(STDIN_BUF_SIZE, char, NULL_TERMINATED);

	ircp_exec_args_t ircp_args = {
		abort, args,
		&readln, NULL
	};

	pthread_t thread;
	pthread_create(&thread, NULL, (threadfunc_t) &ircp_exec, (void*) &ircp_args);

	// main program
	while (!abort(args)) {

		char * line = readline();

		if (!strcmp(line, ":q")) {
			free(line);
			break;
		}

		pthread_mutex_lock(&STDIN_BUFFER_MUTEX);
		rb_push_str(STDIN_BUFFER, line);
		pthread_mutex_unlock(&STDIN_BUFFER_MUTEX);

		free(line);

	}

	LDEBG("Waiting for thread to terminate\n");
	pthread_join(thread, 0);
	rb_free(STDIN_BUFFER);
	cc1200_cmd(SIDLE);
	spi_shutdown();
}

