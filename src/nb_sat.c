#include <pthread.h>                                                   
#include <stdio.h>                                                     
#include <unistd.h>                                                    
#include <inttypes.h>                                                  
#include <stdbool.h>                                                   
#include <Python.h>     //A terme la partie python sera remplacée par du C++
#include <time.h>
#include <signal.h>

void *task(void *arg);

typedef struct info_radio info_radio;
struct info_radio{	//Structure stockant les informations radios personnelles à chaque thread 
        uint32_t freq;
	char name[30];
	char date[30];
	char end_date[30];
};

pthread_mutex_t RTL2832U = PTHREAD_MUTEX_INITIALIZER;	

int main(int argc, char *argv[]){
	uint8_t i,n,nb_sat,nb_rot=0;
	info_radio infos;
	pthread_t thr;

	fprintf(stderr,"Please enter the number of satellites: \n");
	scanf("%"SCNu8,&nb_sat);	//%d ne peut pas être utilisé car enregistre sur 32 bits au lieu de 8
	for(i=0;i<nb_sat;i++){
		fprintf(stderr,"Please enter the name of the satellite \n");
		getchar();	//sert a absorber le \n car scanf ne l'absorbe pas
		fgets(infos.name, 29, stdin);
		fprintf(stderr, "Please enter the frequency \n");
		scanf("%"SCNu32,&(infos.freq));
		fprintf(stderr,"Please enter the number of rotation \n");
		scanf("%"SCNu8,&nb_rot);	
		for(n=0;n<nb_rot;n++){
			fprintf(stderr,"Enter the date of revolution #%c (format = mm-dd-hh-minmin-ss) \n",i);
			getchar();
			fgets(infos.date,29, stdin);
                        fprintf(stderr,"Enter the date of end of revolution #%c (format = mm-dd-hh-minmin-ss) \n",i);
			fgets(infos.end_date,29, stdin);
			fprintf(stderr,"%s \n", infos.end_date);
			if(pthread_create(&thr, NULL, task, &infos) != 0){            
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

/*Pour l'instant la gestion des taches n'est pas très utile car chaque 
 * tâche va lancer un interpreteur python donc une                        
 *  * nouvelle tâche. Cependant à terme la partie Python sera réécrite en 
 *  C++ donc ce sera intéressant d'avoir une gestion                       
 *   * des tâches à ce moment */                                           
void *task(void *arg){                                                 
	char filename[] = "METEO.py";                                   
	FILE *fp;
   	info_radio *infos = (info_radio*) arg;
	uint32_t freq= infos->freq;
	char *date = infos->date;
	char *name = infos->name;
	char *end_date = infos->end_date;
	char sys_date[16];	
        time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	bool LOCK = true;
	int stop_python;
	Py_Initialize();
	PyObject *Module = PyUnicode_FromString("test");
	PyObject *librairie = PyImport_Import(Module);
                                                                              
	while(true){    		
		sleep(1);
		time(&t);
		tm = *localtime(&t);
		sprintf(sys_date,"%02d-%02d-%02d-%02d-%02d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
		fprintf(stderr,"%d \n", LOCK);
		if(LOCK){	//On execute une seule fois l'activation de l'enregistrement 
			if(strcmp(date,sys_date) <= 0){
				pthread_mutex_lock(&RTL2832U);
				fprintf(stderr,"Lancer le soft et prendre mutex \n");
				PyObject* myFunction = PyObject_GetAttrString(librairie,(char*)"main");
				//On passe les arguments à la fonction main python
				//PyObject* args = PyTuple_Pack(2,PyLong_FromLong((long)freq),PyLong_FromString(name,NULL,0));
				//PyObject* args = Py_BuildValue("(is)", freq, name);
				PyObject* args = PyTuple_New(freq);
				PyObject* myResult = PyObject_CallObject(myFunction, args);
				//PyObject* args = Py_BuildValue("(z)",(char*)"something");
				fprintf(stderr,"passer\n");		
				//PrgsyRun_SimpleFile(fp, filename);
				Py_Finalize();
				LOCK = false;
			}
		}
		if(strcmp(end_date,sys_date) <= 0){
			fprintf(stderr,"Stopper le soft et rendre mutex \n");
			stop_python = raise(SIGINT);
			pthread_mutex_unlock(&RTL2832U); 
		}
	}
}
