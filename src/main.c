#include "Config_radio.h"

int main(int argc, char *argv[])
{
	info_radio infs;
	uint8_t nb_sat,i,n=0;
	mqd_t mq;
	pid_t pid;

	logfile(MAIN_PROCESS_NAME,"Opening the message queue");
	mq=mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PRIORITY, NULL);
	if(mq == (mqd_t)-1) {
		perror("open queue ");
		logfile(MAIN_PROCESS_NAME,strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(argc > 1) {
		logfile(MAIN_PROCESS_NAME,"input arguments");
		for(n=1;n<=argc;n++) {
			logfile(MAIN_PROCESS_NAME,"Reading the parameters from the file");
			infs=read_infos(argv[n]);
			logfile(MAIN_PROCESS_NAME,"fork");
			pid=fork();
			//if it is a child process, exit from the loop so as not to create a grandchild process
			if(pid == 0)
				break;
			else {
				send_queue(mq,infs);
			}
		}
	} else {
		logfile(MAIN_PROCESS_NAME,"No input arguments, the user enters the information");
		nb_sat = ask_for_number_sat();

		if(nb_sat == 0) {
			fprintf(stderr,"Leaving the software \n");
			return EXIT_SUCCESS;
		}

		for(i=0;i<nb_sat;i++) {
			infs=manual_config();
			if(infs == 0) {
				fprintf(stderr,"Wrong input, leaving the software \n");
				return EXIT_SUCCESS;
			} else if(infs < 0) {
				fprintf("An error has occured \n");
				return EXIT_FAILURE;
			}
			logfile(MAIN_PROCESS_NAME,"fork");
			pid=fork();
			//if it is a child process, exit from the loop so as not to create a grandchild process
			if(pid == 0)
				break;
			else {
				send_queue(mq,infs);
			}
		}
	}

	if(pid == 0)
		record();

	return EXIT_SUCCESS;
}




