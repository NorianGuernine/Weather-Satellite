#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "Config_radio.h"

info_radio Lecture_infos(char *filename)
{
	/*Lecture du fichier:
	 * 1ere ligne: nom du satellite
	 * 2eme ligne: frequence
	 * 3eme ligne: begin_date de début d"aquisition
	 * 4eme ligne: begin_date de fin d'acquisition*/
	FILE *fp;
	char line[80] = {0};
	info_radio infs;
	fp = fopen(filename,"r");
	if(fp==NULL) {
		exit(EXIT_FAILURE);
	}
	fgets(infs.name, 80, fp);
	infs.freq=(unsigned long)atoi(fgets(line, 80, fp));
	fgets(infs.begin_date, 80, fp);
	fgets(infs.end_date, 80, fp);
	fclose(fp);
	return infs;
}

info_radio Config_manuelle(void)
{
	info_radio infos;
	fprintf(stderr,"Please enter the name of the satellite \n");
	getchar();	//sert a absorber le \n car scanf ne l'absorbe pas
	fgets(infos.name, 29, stdin);
	fprintf(stderr, "Please enter the frequency \n");
	scanf("%lu",&(infos.freq));
	fprintf(stderr,"Enter the begin_date of revolution (format = mm-dd-hh-minmin-ss) \n");
	getchar();
	fgets(infos.begin_date,29, stdin);
	fprintf(stderr,"Enter the begin_date of end of revolution (format = mm-dd-hh-minmin-ss) \n");
	fgets(infos.end_date,29, stdin);
	fprintf(stderr,"%s \n", infos.end_date);

	return infos;
}
