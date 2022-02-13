#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
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
	int result_close_mq;

	mq=mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PRIORITY, NULL);
	if(mq == (mqd_t)-1) {
		perror("Error : ");
		exit(EXIT_FAILURE);
	}

	if(argc > 1) {
		for(n=1;n<=argc;n++) {
			infs=Lecture_infos(argv[n]);
			pid=fork();
			//Si on est dans un process child alors on sort de la boucle pour ne pars créer de grandchild
			if(pid == 0)
				break;
			else {
				if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
					perror("mq_send");
					exit(EXIT_FAILURE);
				}
			}
		}
	} else {
		fprintf(stderr,"Please enter the number of satellites: \n");
		//%d ne peut pas être utilisé car enregistre sur 32 bits au lieu de 8
		scanf("%"SCNu8,&nb_sat);
		for(i=0;i<nb_sat;i++) {
			infs=Config_manuelle();
			pid=fork();
			//Si on est dans un process child alors on sort de la boucle pour ne pars créer de grandchild
			if(pid == 0)
				break;
			else {
				if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
					perror("mq_send");
					exit(EXIT_FAILURE);
				}
			}
		}

	}

	if(pid == 0)
		Enregistrement();
	else {
		result_close_mq = mq_close(mq);
		if(result_close_mq == -1)
			perror("mq_close");
		result_close_mq = mq_unlink(QUEUE_NAME);
		if(result_close_mq == -1)
			perror("mq_unlink");
	}


	return EXIT_SUCCESS;
}




