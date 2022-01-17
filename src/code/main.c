#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <Python.h>
#include <time.h>
#include <signal.h>
#include <string.h>

typedef struct info_radio info_radio;
struct info_radio{	//Structure stockant les informations radios personnelles à chaque thread
    unsigned long freq;
	char name[30];
	char date[30];
	char end_date[30];
};

void *task(void *arg);	//Thread qui enregistre le données satellite
info_radio Lecture_infos(char *filename);	//Fonction qui lit les paramètres enregistrés dans des fichiers
info_radio Config_manuelle(void);

pthread_mutex_t RTL2832U = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
	info_radio infs;
	pthread_t thr;
	uint8_t nb_sat,i,n=0;
	if(argc > 1) {
		for(n=1;n<=argc;n++) {
			infs=Lecture_infos(argv[i]);
			if(pthread_create(&thr, NULL, task, &infs) != 0){
				fprintf(stderr, "Error during pthread_create() \n");
				exit(EXIT_FAILURE);
				return -1;
			}
		}
	}
	else {
		fprintf(stderr,"Please enter the number of satellites: \n");
		scanf("%"SCNu8,&nb_sat);	//%d ne peut pas être utilisé car enregistre sur 32 bits au lieu de 8
		for(i=0;i<nb_sat;i++) {
			infs=Config_manuelle();
			if(pthread_create(&thr, NULL, task, &infs) != 0){
				fprintf(stderr, "Error during pthread_create() \n");
				exit(EXIT_FAILURE);
				return -1;
			}
		}
	}
	while(true){
		sleep(1);
	}
	return 0;
}

void *task(void *arg){
   	info_radio *infos = (info_radio*) arg;
	unsigned long freq= infos->freq;
	char *date = infos->date;
	char *name = infos->name;
	char *end_date = infos->end_date;
	char sys_date[16];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	Py_Initialize();
	PyObject *pName, *pModule, *pFunc, *pArgs,*pValue;
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* path = PyObject_GetAttrString(sys, "path");
	PyList_Insert(path, 0, PyUnicode_FromString("."));

	do{	//On attend le début de la date d'enregistrement
		sleep(1);
		time(&t);
		tm = *localtime(&t);
		fprintf(stderr,"%s",infos->name);
		sprintf(sys_date,"%02d-%02d-%02d-%02d-%02d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	}while(strcmp(date,sys_date) > 0);
	//On enregistre les données satellite en lançant le soft python
	pthread_mutex_lock(&RTL2832U);
	fprintf(stderr,"Lancer le soft et prendre mutex \n");
	pName = PyUnicode_FromString((char*)"Meteo");
	pModule = PyImport_Import(pName);
	pFunc = PyObject_GetAttrString(pModule, (char*)"main");
	pArgs = Py_BuildValue("(ssk)",name,end_date,freq);
	pValue = PyObject_CallObject(pFunc, pArgs);
	Py_Finalize();
	//Quand la date de fin d'enregistrement est atteinte le soft python rend la main au programme C
	fprintf(stderr,"Stopper le soft et rendre mutex \n");
	pthread_mutex_unlock(&RTL2832U);
	return 0;
}

info_radio Lecture_infos(char *filename) {
	/*Lecture du fichier:
	 * 1ere ligne: nom du satellite
	 * 2eme ligne: frequence
	 * 3eme ligne: date de début d"aquisition
	 * 4eme ligne: date de fin d'acquisition*/
	FILE *fp;
	char line[80] = {0};
	info_radio infs;
	fp = fopen(filename,"r");
	if(fp==NULL) {
		exit(EXIT_FAILURE);
	}
	fgets(infs.name, 80, fp);
	infs.freq=(unsigned long)atoi(fgets(line, 80, fp));
	fgets(infs.date, 80, fp);
	fgets(infs.end_date, 80, fp);
	fclose(fp);
	return infs;
}

info_radio Config_manuelle(void) {
	info_radio infos;

	fprintf(stderr,"Please enter the name of the satellite \n");
	getchar();	//sert a absorber le \n car scanf ne l'absorbe pas
	fgets(infos.name, 29, stdin);
	fprintf(stderr, "Please enter the frequency \n");
	scanf("%lu",&(infos.freq));
	fprintf(stderr,"Enter the date of revolution (format = mm-dd-hh-minmin-ss) \n");
	getchar();
	fgets(infos.date,29, stdin);
	fprintf(stderr,"Enter the date of end of revolution (format = mm-dd-hh-minmin-ss) \n");
	fgets(infos.end_date,29, stdin);
	fprintf(stderr,"%s \n", infos.end_date);

	return infos;
}


