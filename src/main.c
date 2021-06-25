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
#include <getopt.h>
# include <unistd.h>

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

#define RX 0
#define TX 1
 

// https://stackoverflow.com/a/37631288/14661040
volatile sig_atomic_t sigint_received = 0;

void sigint_handler(int s) {
	LINFO("Received SIGINT: %d\n", s);
	sigint_received = 1;
}

void program_chat(){

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

	return ;

}


void program_rssi(char mode){
	cc1200_init_reg(RegSettings, ExtRegSettings);

	
	if (mode == RX){
		cc1200_cmd(SRX);

		signed char rssi = 0;
		while (true) {
			rssi = (signed char) cc1200_reg_read(RSSI1, 0);
			printf("INFO: CC1200: RSSI1: %d\n", (int) rssi);
			
			sleep(1);
		}
	}else{
		cc1200_cmd(STX);
		while (true) {
			sleep(1);

		}
	}

		spi_shutdown();
	
		return;
}
char check_param(char* optarg ){
	char param = RX; 
	printf("check_param : input  %s \n", optarg);
	if (strncasecmp(optarg,"=rx",3)== 0){
		param = RX;
	}

	if (strncasecmp(optarg,"=tx",3)==0){
		param = TX;
	}
	
	printf("check_param: param: %d \n" ,param );
	return param;
}

void programm_interfier(){}

int main (int argc, char **argv) {



	// prepare sigint_handler
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);

	static struct option long_options[] = 
		{
			{"rssi",required_argument, 0, 'r' },
			{"interfere", no_argument, 0 , 'i' },
			{"chat",no_argument,0,'c'},
			{"help", no_argument,0, 'h'},
			{0, 0, 0, 0} 
		};

	//program_rssi(boolen mode(rx/tx)) 
	//program_chatt()
	//programm_interfier()
	//help


	int c;

	while(1){

		 /* getopt_long stores the option index here. */
    	int option_index = 0;

    	c = getopt_long (argc, argv, "r:ich",
                       long_options, &option_index);
		
		  /* Detect the end of the options. */
    	if (c == -1){break;}
		switch (c)
		{
		case 0:

		case 'c' :
			LINFO("executing chat program\n");
			program_chat();
			break;
		case 'r':
			LINFO("executing rssi program\n");
			program_rssi(check_param(optarg));
			break;
		
		case 'i':
			LINFO("executing interfere program\n");
			break;
		
		case 'h':
			printf("sage: <program> [OPTION]... OPTION -c, --chat main chat program -i, --interfere interfere program -r, --rssi=MODE meassure (MODE=rx) or send (MODE=tx) rssi -h, --help print this screen \n");
			break;

		default:
			LINFO("executing chat program\n");
			program_chat();
			break;
		}
        
	}


	spi_shutdown();
	return 0;

	}
