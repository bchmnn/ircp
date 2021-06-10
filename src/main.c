/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include <external/SPIv1.h>

#include "cc1200_reg.h"
#include "cc1200_preinit.h"
#include "cc1200_function.h"
#include "cc1200_thread.h"
#include "util/ringbuffer.h"
#include "util/readline.h"
#include "util/log.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

#define BUF_SIZE 2048

// https://stackoverflow.com/a/37631288/14661040
volatile sig_atomic_t sigint_received = 0;

void sigint_handler(int s) {
	LINFO("Received SIGINT: %d\n", s);
	sigint_received = 1;
}

int main (void) {

	// prepare sigint_handler
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);
 
	cc1200_init_reg(RegSettings, ExtRegSettings);

	// initialize thread
	rb_t* buf = rb_init(BUF_SIZE, char, NULL_TERMINATED);
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
	while (!sigint_received) {

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
	pthread_join(thread, 0);
	rb_free(buf);
	spi_shutdown();

	return 0;
}
