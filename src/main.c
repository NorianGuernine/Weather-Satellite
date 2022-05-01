#include "config_radio.h"

/* Cannot give argument to signal function, for this case global variable seems to be an acceptable solution */
extern sig_atomic_t sig_child;

int main(int argc, char *argv[])
{
	info_radio infs;
	uint8_t i,n, nb_process_alive = 0;
	uint8_t nb_sat = 0;
	uint16_t time_since_beginning = 0;
	uint32_t watchdog = 0; // watchdog in minutes
	bool end_of_process, config_file_ok = false;
	mqd_t mq;
	pid_t pid;
	int return_manual_config, return_read_infos, is_param, want_watchdog = 0;
	char msg_to_log[NB_MAX_CHARACTERS];


	logfile(MAIN_PROCESS_NAME,"Opening the message queue");
	mq=mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PRIORITY, NULL);
	if(mq == (mqd_t)-1) {
		perror("open queue ");
		logfile(MAIN_PROCESS_NAME,strerror(errno));
		return EXIT_FAILURE;
	}

	if(argc > 1) {
		logfile(MAIN_PROCESS_NAME,"input arguments");
		for(n=1;n<argc;n++) {
			logfile(MAIN_PROCESS_NAME,"Reading the parameters from the file");
			/* Check for parameters such as watchdog */
			is_param = check_if_param(argv[n]);
			if(is_param < 0) {
				logfile(MAIN_PROCESS_NAME,"Read argument return error, the program will exit now");
				return EXIT_FAILURE;
			} else if(is_param == 1) {
				n++;
				watchdog = atoi(argv[n]);
			} else {
				return_read_infos = read_infos(&infs,argv[n]);
				if(return_read_infos < 0) {
					fprintf(stderr,"Leaving the software \n");
					logfile(MAIN_PROCESS_NAME,"An error has occured, leaving the software \n");
				}
				logfile(MAIN_PROCESS_NAME,"fork");
				pid=fork();
				//if it is a child process, exit from the loop so as not to create a grandchild process
				if(pid == 0)
					break;
				else {
					send_queue(mq,infs);
				}
				/* set flag to 1 to indicate at least one config file read */
				config_file_ok = true;
			}

		}

		if( !config_file_ok ) {
			logfile(MAIN_PROCESS_NAME,"No configuration file read, error");
			fprintf(stderr,"No configuration file read, error \n");
			return EXIT_FAILURE;
		}
} else {
		logfile(MAIN_PROCESS_NAME,"No input arguments, the user enters the information");
		nb_sat = ask_for_number_sat();
		want_watchdog = ask_for_watchdog();
		if( want_watchdog < 0 ) {
			fprintf(stderr, "Leaving the software \n");
			return EXIT_SUCCESS;
		} else if( want_watchdog == 1 ) {
			watchdog = set_watchdog();
			if( watchdog == 0 ) {
				fprintf(stderr, "Leaving the software \n");
				return EXIT_FAILURE;
			}
		}

		if(nb_sat == 0) {
			fprintf(stderr,"Leaving the software \n");
			return EXIT_SUCCESS;
		}

		for(i=0;i<nb_sat;i++) {
			return_manual_config = manual_config(&infs);

			if(return_manual_config < 0) {
				fprintf(stderr,"Leaving the software \n");
				return EXIT_FAILURE;
			}
			logfile(MAIN_PROCESS_NAME,"fork");
			pid=fork();
			/* if it is a child process, exit from the loop so as not to create a grandchild process */
			if(pid == 0)
				break;

			else {
				/* increment the counter of child process */
				nb_process_alive++;
				send_queue(mq,infs);
			}
		}
	}

	if(pid == 0) {
		record();

	} else {
		/* In the parent process */
		if(nb_process_alive != nb_sat)
			/* nb_process_alive should be equal to nb_sat */
			logfile(MAIN_PROCESS_NAME, "Number of process create different from number of satellite");

		while(true) {

			signal ( SIGCHLD, catch_child_signal );

			if( sig_child == 1) {
				sig_child = 0;
				nb_process_alive--;
				end_of_process = true;
			}

			sprintf(msg_to_log,"Number of child process alive %d", nb_process_alive);

			if(end_of_process) {
				logfile(MAIN_PROCESS_NAME,msg_to_log);
				end_of_process = false;
			}

			/* Check if time in minute = watchdog, if so, power off the system */
			if( watchdog > 0 ) {
				if( time_since_beginning / 60 == watchdog ) {
					logfile(MAIN_PROCESS_NAME,"Watchdog, the system will now power off");
				}
			}
			sleep(1);
			time_since_beginning++;
		}

		logfile(MAIN_PROCESS_NAME,"End of all the records, the system will now shut down");
	}

	return EXIT_SUCCESS;
}




