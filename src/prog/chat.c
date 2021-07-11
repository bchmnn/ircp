#include "prog/chat.h"

#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <external/SPIv1.h>

#include "cc1200/config.h"
#include "cc1200/control.h"
#include "cc1200/freq.h"
#include "cc1200/utils.h"
#include "util/log.h"
#include "util/ringbuffer.h"
#include "util/readline.h"
#include "config.h"
#include "cc1200_thread.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

void pchat(bool(*abort)(void*), void* args) {

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
	rb_t* buf = rb_init(STDIN_BUF_SIZE, char, NULL_TERMINATED);
	static pthread_mutex_t buf_mutex;
	static bool term_signal = false;
	static pthread_mutex_t term_signal_mutex;

	cc1200_thread_args_t thread_args = {
		buf,          &buf_mutex,
		&term_signal, &term_signal_mutex
	};

	pthread_t thread;
	pthread_create(&thread, NULL, cc1200_thread, (void*) &thread_args);

	// main program
	while (!abort(args)) {

		char * line = readline();

		if (!strcmp(line, ":q")) {
			free(line);
			break;
		}

		pthread_mutex_lock(&buf_mutex);
		rb_push_str(buf, line);
		pthread_mutex_unlock(&buf_mutex);

		free(line);

	}

	set_term_signal(&thread_args);
	LDEBG("Waiting for thread to terminate\n");
	pthread_join(thread, 0);
	rb_free(buf);
	cc1200_cmd(SIDLE);
	spi_shutdown();
}

