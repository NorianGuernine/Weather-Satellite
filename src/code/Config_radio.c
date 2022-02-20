#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <Python.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>
#include <semaphore.h>
#include <fcntl.h>
#include "Config_radio.h"

info_radio Lecture_infos(char *filename)
{
	/*Lecture du fichier:
	 * 1ere ligne: nom du satellite
	 * 2eme ligne: frequence
	 * 3eme ligne: begin_date de début d"aquisition
	 * 4eme ligne: end_date de fin d'acquisition*/

	FILE *fp;
	char line[80] = {0};
	info_radio infs;
	fp = fopen(filename,"r");
	logfile(MAIN_PROCESS_NAME,"Attempt to read parameter files");
	if(fp==NULL) {
		logfile(MAIN_PROCESS_NAME,"Attempting to read parameter files returns null");
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
	fprintf(stderr,"Enter the date of revolution (format = mm-dd-hh-minmin-ss) \n");
	getchar();
	fgets(infos.begin_date,29, stdin);
	fprintf(stderr,"Enter the date of end of revolution (format = mm-dd-hh-minmin-ss) \n");
	fgets(infos.end_date,29, stdin);

	return infos;
}
int Enregistrement(void)
{
	char sys_date[16];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	mqd_t mq;
	int test_mq_receive;
	char buffer[SIZE_INFO_RADIO];
	char msg_to_log[SIZE_MSG_TO_LOG];
	char string_pid_number[10];
	unsigned int priority;
	struct mq_attr attr;
	sem_t *RTL2832U;
	pid_t pid;

	pid = getpid();
	//On stocke le numéro du PID dans string_pid_number
	sprintf(string_pid_number,"[%d]",pid);
	logfile(string_pid_number,"Attempt to open message queue");
	mq=mq_open(QUEUE_NAME, O_RDONLY);
	if(mq == (mqd_t) -1) {
		perror("queue");
		logfile(string_pid_number,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(mq_getattr(mq, & attr) != 0) {
		perror("mq_getattr");
		logfile(string_pid_number,strerror(errno));
		exit(EXIT_FAILURE);
	}

	logfile(string_pid_number,"Attempt to receive message queue with info_radio elements");
	test_mq_receive = mq_receive(mq,buffer,attr.mq_msgsize, &priority);
	if(test_mq_receive < 0) {
		perror("mq_receive");
		logfile(string_pid_number,strerror(errno));
		exit(EXIT_FAILURE);
	}
	info_radio *infs = (info_radio *)buffer;
	//Ce message vers le log est enregistré dans une variable pour y rajouter le nom du fichier
	sprintf(msg_to_log,"This process write to the file %s", infs->name);
	logfile(string_pid_number,msg_to_log);

	/*On commence par essayer d'avoir accès à la semaphore
	 * si on ne peut pas alors on la crée*/

	logfile(string_pid_number,"Attempt to get acces to the semaphore");

	RTL2832U = sem_open(SEMAPHORE_NAME, O_RDWR);
	if(RTL2832U == SEM_FAILED) {
		if(errno != ENOENT) {
			perror(SEMAPHORE_NAME);
			logfile(string_pid_number,strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	RTL2832U = sem_open(SEMAPHORE_NAME, O_RDWR | O_CREAT, SEMAPHORE_PERMISSION, 1);
	if(RTL2832U == SEM_FAILED) {
		perror(SEMAPHORE_NAME);
		logfile(string_pid_number,strerror(errno));
		exit(EXIT_FAILURE);
	}
	logfile(string_pid_number,"Waiting for the semaphore");
	sem_wait(RTL2832U);
	logfile(string_pid_number,"Taking the semaphore");


	Py_Initialize();
	PyObject *pName, *pModule, *pFunc, *pArgs,*pValue;
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* path = PyObject_GetAttrString(sys, "path");
	PyList_Insert(path, 0, PyUnicode_FromString("."));

	//On attend le début de la begin_date d'enregistrement
	logfile(string_pid_number,"Waiting for the recording date");
	do {
		sleep(1);
		time(&t);
		tm = *localtime(&t);
		sprintf(sys_date,"%02d-%02d-%02d-%02d-%02d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	} while(strcmp(infs->begin_date,sys_date) > 0);

	//On enregistre les données satellite en lançant le soft python
	logfile(string_pid_number,"Configuring the python interpreter");
	pName = PyUnicode_FromString((char*)"Meteo");
	if(pName == NULL) {
		fprintf(stderr,"pName in python (PyUnicode_FromString) configuration return NULL");
		logfile(string_pid_number,"pName (PyUnicode_FromString) in python configuration return NULL");
		exit(EXIT_FAILURE);
	}
	pModule = PyImport_Import(pName);
	if(pModule == NULL) {
		fprintf(stderr,"pModule in python (PyImport_Import) configuration return NULL");
		logfile(string_pid_number,"pModule in python (PyImport_Import) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	pFunc = PyObject_GetAttrString(pModule, (char*)"main");
	if(pFunc == NULL) {
		fprintf(stderr,"pFunc in python (PyObject_GetAttrString) configuration return NULL");
		logfile(string_pid_number,"pFunc in python (PyObject_GetAttrString) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	pArgs = Py_BuildValue("(ssi)",infs->name,infs->end_date,infs->freq);
	if(pArgs == NULL) {
		fprintf(stderr,"pArgs in python (Py_BuildValue) configuration return NULL");
		logfile(string_pid_number,"pArgs in python (Py_BuildValue) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	pValue = PyObject_CallObject(pFunc, pArgs);
	if(pValue == NULL) {
		fprintf(stderr,"pValue in python (PyObject_CallObject) configuration return NULL");
		logfile(string_pid_number,"pValue in python (PyObject_CallObject) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	logfile(string_pid_number,"Stopping the python interpreter");
	Py_Finalize();

	//Quand la begin_date de fin d'enregistrement est atteinte le soft python rend la main au programme C
	sem_post(RTL2832U);
	logfile(string_pid_number,"Semaphore released");

	return EXIT_SUCCESS;
}

int logfile(char * what_process, char * msg)
{
	time_t t = time(NULL);
	struct tm tm;
	int logfile;
	struct flock lock;
	size_t size_msg, size_msg_to_write;
	char *msg_to_write = NULL;
	char date_logfile[SIZE_DATE_LOGFILE];

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	logfile=open("Weather_sat.log", O_CREAT|O_WRONLY|O_APPEND);

	while(fcntl(logfile, F_SETLK, &lock) < 0)
		if(errno != EINTR)
			return -1;

	//Si accès réussie
	tm = *localtime(&t);

	sprintf(date_logfile,"[%02d:%02d:%02d:%02d:%02d] ",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	assert(msg_to_write == NULL);
	//+3 pour prendre en compte les deux \n et le \0 dans le message final
	size_msg = strlen(date_logfile) + strlen(what_process) + strlen(msg) + 3;
	msg_to_write = (char *) malloc(size_msg * sizeof(char));
	if(msg_to_write == NULL) {
		fprintf(stderr,"Not enough memory for malloc \n");
		exit(EXIT_FAILURE);
	}
	assert(msg_to_write != NULL);
	sprintf(msg_to_write,"%s%s%s\n\n", date_logfile, what_process, msg);
	//Pour la taille du message size_msg_to_write on ne prend pas en compte le char \0 à la fin
	size_msg_to_write = strlen(msg_to_write);
	write(logfile, msg_to_write, size_msg_to_write * sizeof(char));
	close(logfile);
	fcntl(logfile, F_UNLCK, &lock);

	//free memory
	free(msg_to_write);

	return EXIT_SUCCESS;
}



