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
#include "Config_radio.h"


int main(void)
{
	char sys_date[16];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	mqd_t mq;
	int test_mq_receive;
	char buffer[SIZE_INFO_RADIO];
	unsigned int priority;
	struct mq_attr attr;
	sem_t *RTL2832U;

	mq=mq_open(QUEUE_NAME, O_RDONLY);
	if(mq == (mqd_t) -1) {
		perror("queue");
		exit(EXIT_FAILURE);
	}
	if(mq_getattr(mq, & attr) != 0) {
		perror("mq_getattr");
		exit(EXIT_FAILURE);
	}

	test_mq_receive = mq_receive(mq,buffer,attr.mq_msgsize, &priority);
	if(test_mq_receive < 0) {
		perror("mq_send");
		exit(EXIT_FAILURE);
	}
	info_radio *infs = (info_radio *)buffer;

	RTL2832U = sem_open(SEMAPHORE_NAME, O_RDWR);
	if(RTL2832U == SEM_FAILED) {
		if(errno != ENOENT) {
			perror(SEMAPHORE_NAME);
			exit(EXIT_FAILURE);
		}
	}
	RTL2832U = sem_open(SEMAPHORE_NAME, O_RDWR | O_CREAT, SEMAPHORE_PERMISSION, 1);

	Py_Initialize();
	PyObject *pName, *pModule, *pFunc, *pArgs,*pValue;
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* path = PyObject_GetAttrString(sys, "path");
	PyList_Insert(path, 0, PyUnicode_FromString("."));

	do {
		sleep(1);
		time(&t);
		tm = *localtime(&t);
		sprintf(sys_date,"%02d-%02d-%02d-%02d-%02d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	} while(strcmp(infs->begin_date,sys_date) > 0);	//On attend le début de la begin_date d'enregistrement
	//On enregistre les données satellite en lançant le soft python
	fprintf(stderr,"Lancer le soft et prendre mutex \n");
	pName = PyUnicode_FromString((char*)"Meteo");
	pModule = PyImport_Import(pName);
	pFunc = PyObject_GetAttrString(pModule, (char*)"main");
	pArgs = Py_BuildValue("(ssi)",infs->name,infs->end_date,infs->freq);
	pValue = PyObject_CallObject(pFunc, pArgs);
	Py_Finalize();
	//Quand la begin_date de fin d'enregistrement est atteinte le soft python rend la main au programme C
	fprintf(stderr,"Stopper le soft et rendre mutex \n");

	return EXIT_SUCCESS;
}



