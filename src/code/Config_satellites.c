#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "Config_radio.h"
#include <mqueue.h>

int main(int argc, char *argv[])
{
	info_radio infs;
	uint8_t nb_sat,i,n=0;
	mqd_t mq;

	mq=mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PRIORITY, NULL);
	if(mq == (mqd_t)-1) {
		perror("Error : ");
		exit(EXIT_FAILURE);
	}

	if(argc > 1) {
		for(n=1;n<=argc;n++) {
			infs=Lecture_infos(argv[n]);
			//execv("/home/norian/Documents/github/Satellite_Meteo/Debug/Satellite_Meteo",NULL);
			if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
		}
	} else {
		fprintf(stderr,"Please enter the number of satellites: \n");
		scanf("%"SCNu8,&nb_sat);	//%d ne peut pas être utilisé car enregistre sur 32 bits au lieu de 8
		for(i=0;i<nb_sat;i++) {
			infs=Config_manuelle();
			//execv("/home/norian/Documents/github/Satellite_Meteo/Debug/Satellite_Meteo",NULL);
			if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
		}
	}

	return EXIT_SUCCESS;
}
