#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include "prog/prssi.h"
#include "prog/chat.h"
#include "util/log.h"
#include "util/types.h"

#define LOGGING_LEVEL TRACE
#define LERR(fmt, ...) _LOG_ERROR(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LWARN(fmt, ...) _LOG_WARN(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LINFO(fmt, ...) _LOG_INFO(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LDEBG(fmt, ...) _LOG_DEBUG(LOGGING_LEVEL, fmt, ##__VA_ARGS__)
#define LTRAC(fmt, ...) _LOG_TRACE(LOGGING_LEVEL, fmt, ##__VA_ARGS__)

// https://stackoverflow.com/a/37631288/14661040
volatile sig_atomic_t sigint_received = 0;

void sigint_handler(int s) {
	printf("Received SIGINT: %d\n", s);
	sigint_received = 1;
}

sig_atomic_t get_sigint_received() {
	return sigint_received;
}

void usage() {
	printf("usage: <program> [OPTION]...\n\n");
	printf("OPTION\n");
	printf("-c, --chat       main chat program\n");
	printf("-i, --interfere  interfere program\n");
	printf("-r, --rssi=MODE  meassure (MODE=rx) or send (MODE=tx) rssi\n");
	printf("-h, --help       print this screen");
}

prssi_mode_t get_prssi_optarg(char* optarg) {
	if (strncasecmp(optarg,"rx", 2) == 0)
		return PRSSI_RX;
	else if (strncasecmp(optarg,"tx", 2) == 0)
		return PRSSI_TX;
	else
		return -1;
}

u_int32_t get_freq_optarg(char* optarg) {
	u_int32_t freq = 0;
	char c = *optarg;
	while (c) {
		freq = (freq*10) + (u_int32_t) (c - '0');
		c = *++optarg;
	}
	return freq;
}

int main (int argc, char **argv) {

	// prepare sigint_handler
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);

	static struct option long_options[] = {
		{"rssi",      required_argument, 0, 'r'},
		{"freq",      required_argument, 0, 'f'},
		{"interfere", no_argument,       0, 'i'},
		{"chat",      no_argument,       0, 'c'},
		{"help",      no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};

	int c;
	bool exec_pchat         = true;
	prssi_mode_t prssi_mode = 0;
	bool exec_prssi         = false;
	u_int32_t freq          = 0;

	while (true) {

		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long(argc, argv, "r:f:ich", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 0:
			case 'c' :
				exec_pchat = true;
				exec_prssi = false;
				break;
			case 'r':
				exec_pchat = false;
				exec_prssi = true;
				prssi_mode = get_prssi_optarg(optarg);
				break;
			case 'f':
				freq = get_freq_optarg(optarg);
				break;
			case 'i':
				LERR("Not implemented\n");
				break;
			case 'h':
				usage();
				return 0;
			default:
				break;
		}
	}
	if (exec_pchat) {
		LINFO("Executing chat program\n");
		pchat((boolfunc_t) &get_sigint_received, NULL);
	} else if (exec_prssi) {
		LINFO("Executing rssi program\n");
		prssi(prssi_mode, freq, (boolfunc_t) &get_sigint_received, NULL);
	}

	return 0;

}
