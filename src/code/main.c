#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <mqueue.h>
#include <semaphore.h>
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
			infs=Lecture_infos(argv[n]);
			logfile(MAIN_PROCESS_NAME,"fork");
			pid=fork();
			//Si on est dans un process child alors on sort de la boucle pour ne pars créer de grandchild
			if(pid == 0)
				break;
			else {
				logfile(MAIN_PROCESS_NAME,"Attempt to send message queue infs");
				if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
					perror("mq_send");
					logfile(MAIN_PROCESS_NAME,strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
		}
	} else {
		logfile(MAIN_PROCESS_NAME,"No input arguments, the user enters the information");
		fprintf(stderr,"Please enter the number of satellites: \n");
		//%d ne peut pas être utilisé car enregistre sur 32 bits au lieu de 8
		scanf("%"SCNu8,&nb_sat);
		for(i=0;i<nb_sat;i++) {
			infs=Config_manuelle();
			logfile(MAIN_PROCESS_NAME,"fork");
			pid=fork();
			//Si on est dans un process child alors on sort de la boucle pour ne pars créer de grandchild
			if(pid == 0)
				break;
			else {
				logfile(MAIN_PROCESS_NAME,"Attempt to send message queue infs");
				if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
					perror("mq_send");
					logfile(MAIN_PROCESS_NAME,strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
		}

	}

	if(pid == 0)
		Enregistrement();


	return EXIT_SUCCESS;
}




