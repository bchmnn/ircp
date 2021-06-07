/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <SPIv1.h>

#include "cc1200_reg.h"
#include "cc1200_preinit.h"
#include "cc1200_function.h"
#include "readline.h"

#define BUF_SIZE 2048

char buf[BUF_SIZE];
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;

bool terminated = false;
pthread_mutex_t term_mutex = PTHREAD_MUTEX_INITIALIZER;

void terminate_threads() {
	pthread_mutex_lock(&term_mutex);
	terminated = true;
	pthread_mutex_unlock(&term_mutex);
}

bool is_terminated() {
	pthread_mutex_lock(&term_mutex);
	bool ret = terminated;
	pthread_mutex_unlock(&term_mutex);
	return ret;
}

void sigint_handler(int status) {
	printf("\nWARN: Received SIGINT: %d\n", status);

	terminate_threads();
	pthread_join(0, NULL);
	
	printf("INFO: Shutting down SPI...\n");
	spi_shutdown();

	exit(1);
}

void *cc1200_thread() {
	while (true) {
		pthread_mutex_lock(&buf_mutex);
		if (buf[0]) {
			int len = strlen(buf);
			cc1200_tx(buf, len);
			buf[0] = '\0';
		}
		pthread_mutex_unlock(&buf_mutex);
		char* packet = cc1200_rx(cc1200_rx_preparar());
		if (strlen(packet) > 0)
			printf("%s\n", packet);
		free(packet);
		if (is_terminated())
			pthread_exit(0);
		usleep(10);
	}
}

int main (void) {
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);
 
	cc1200_init_reg(RegSettings, ExtRegSettings);

	pthread_t thread;
	pthread_create(&thread, NULL, cc1200_thread, NULL);

	while (true) {
		char * line = readline();

		pthread_mutex_lock(&buf_mutex);
		strcpy(buf, line);
		pthread_mutex_unlock(&buf_mutex);

		free(line);
	}

	sigint_handler(0);

	return 0;
}
