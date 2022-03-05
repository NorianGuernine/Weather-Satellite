#include "Config_radio.h"

info_radio read_infos(char *filename)
{
	/*Reading the file:
	 * 1st line: satellite name
	 * 2nd line: frequency
	 * 3rd line: acquisition start date
	 * 4th line: acquisition end date*/

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

info_radio manual_config(void)
{
	info_radio infos;
	char string[NB_MAX_CHARACTERS] = "";
	bool good_frequency = false;

	do {
		fprintf(stderr,"Please enter the name of the satellite \n");
		input(infos.name);

		if(strlen(infos.name) <= 1) {
			fprintf(stderr,"Not enough characters \n");
			if(!ask_if_enter_again()) {
				infos.name[0] = '\0';
				return infos;
			}
		}
	} while(strlen(infos.name) <= 1);

	do {
		fprintf(stderr, "Please enter the frequency \n");
		input(string);
		if(sscanf(string, "%lu", &(infos.freq)) != 1) {
			fprintf(stderr,"Incorrect value \n");
			if(ask_if_enter_again()) {
				infos.freq = 0;
				return infos;
			}
		}
		else
			good_frequency = true;
	} while(!good_frequency);

	fprintf(stderr,"Enter the date of revolution (format = mm-dd-hh-minmin-ss) \n");
	ask_for_date(infos.begin_date);
	fprintf(stderr,"Enter the date of end of revolution (format = mm-dd-hh-minmin-ss) \n");
	ask_for_date(infos.end_date);

	return infos;
}
int record(void)
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
	//storing PID number in string_pid_number
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
	//This message to the log is saved in a variable to add the file name
	sprintf(msg_to_log,"This process write to the file %s", infs->name);
	logfile(string_pid_number,msg_to_log);

	//try to get access to semaphore and if not possible then create it

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
	PyObject *python_name, *python_module, *python_func, *python_args,*python_value;
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* path = PyObject_GetAttrString(sys, "path");
	PyList_Insert(path, 0, PyUnicode_FromString("."));

	//Waiting for the recording date stored in begin_date
	logfile(string_pid_number,"Waiting for the recording date");
	do {
		sleep(1);
		time(&t);
		tm = *localtime(&t);
		sprintf(sys_date,"%02d-%02d-%02d-%02d-%02d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	} while(strcmp(infs->begin_date,sys_date) > 0);

	/*Recording of satellite data, the recording of satellite data is done in the python part.*/
	logfile(string_pid_number,"Configuring the python interpreter");
	python_name = PyUnicode_FromString((char*)"Meteo");
	if(python_name == NULL) {
		fprintf(stderr,"pName in python (PyUnicode_FromString) configuration return NULL");
		logfile(string_pid_number,"pName (PyUnicode_FromString) in python configuration return NULL");
		exit(EXIT_FAILURE);
	}
	python_module = PyImport_Import(python_name);
	if(python_module == NULL) {
		fprintf(stderr,"pModule in python (PyImport_Import) configuration return NULL");
		logfile(string_pid_number,"pModule in python (PyImport_Import) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	python_func = PyObject_GetAttrString(python_module, (char*)"main");
	if(python_func == NULL) {
		fprintf(stderr,"pFunc in python (PyObject_GetAttrString) configuration return NULL");
		logfile(string_pid_number,"pFunc in python (PyObject_GetAttrString) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	python_args = Py_BuildValue("(ssi)",infs->name,infs->end_date,infs->freq);
	if(python_args == NULL) {
		fprintf(stderr,"pArgs in python (Py_BuildValue) configuration return NULL");
		logfile(string_pid_number,"pArgs in python (Py_BuildValue) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	python_value = PyObject_CallObject(python_func, python_args);
	if(python_value == NULL) {
		fprintf(stderr,"pValue in python (PyObject_CallObject) configuration return NULL");
		logfile(string_pid_number,"pValue in python (PyObject_CallObject) configuration return NULL");
		exit(EXIT_FAILURE);
	}
	logfile(string_pid_number,"Stopping the python interpreter");
	Py_Finalize();

	//When the end date of recording is reached, the python software returns control to the C program
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

	//Si accès successfull
	tm = *localtime(&t);

	sprintf(date_logfile,"[%02d:%02d:%02d:%02d:%02d] ",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	assert(msg_to_write == NULL);
	//+3 for considering both \n and the \0 in the end of message
	size_msg = strlen(date_logfile) + strlen(what_process) + strlen(msg) + 3;
	msg_to_write = (char *) malloc(size_msg * sizeof(char));
	if(msg_to_write == NULL) {
		fprintf(stderr,"Not enough memory for malloc \n");
		exit(EXIT_FAILURE);
	}
	assert(msg_to_write != NULL);
	sprintf(msg_to_write,"%s%s%s\n\n", date_logfile, what_process, msg);
	//For the size_msg_to_write message size, ignore the char \0 at the end
	size_msg_to_write = strlen(msg_to_write);
	write(logfile, msg_to_write, size_msg_to_write * sizeof(char));
	close(logfile);
	fcntl(logfile, F_UNLCK, &lock);

	//free memory
	free(msg_to_write);

	return EXIT_SUCCESS;
}

void send_queue(mqd_t mq, info_radio infs)
{
	logfile(MAIN_PROCESS_NAME,"Attempt to send message queue infs");
	if(mq_send(mq, (const char *) &infs, sizeof(infs), QUEUE_PRIORITY) != 0) {
		perror("mq_send");
		logfile(MAIN_PROCESS_NAME,strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int ask_for_date(char *string)
{
	bool format_date_ok = false;
	int return_input_date = 0;
	int day, month, hour, min = 100;
	do {
		input(string);
		return_input_date = sscanf(string, "%d %d %d %d", &month, &day, &hour, &min);

		if(return_input_date != 4 || strlen(string) != 11 || month < 1 || month > 12 || day < 1 || day > 31 || hour > 24 || min > 60) {
			if(!ask_if_enter_again()) {
				string[0] = '\0';
				return 0;
			}
			//If ask_if_enter_again does not return 0 then the user wants to re-enter the date
			fprintf(stderr,"Please enter again the date \n");
		}
		else
			format_date_ok = true;

	} while(!format_date_ok);

	return 1;
}

uint8_t ask_for_number_sat(void)
{
	uint8_t nb_sat = 0;
	char char_nb_sat[NB_MAX_CHARACTERS]="";
	bool input_nb_sat_ok = false;

	while(!input_nb_sat_ok) {
		fprintf(stderr,"Please enter the number of satellites: \n");
		//%d cannot be used because recording on 32 bits instead of 8
		input(char_nb_sat);
		if(sscanf(char_nb_sat, "%"SCNu8, &nb_sat) != 1) {
			if(!ask_if_enter_again())
				return 0;
		}

		else
			input_nb_sat_ok = true;
	}
	return nb_sat;
}


void input(char *string)
{
	/*this function is used to replace the \n by a \0 at the end of an entry.
	 * This prevents missing an entry on the next request.*/

    int i = 0;

    if(fgets(string,NB_MAX_CHARACTERS,stdin) != NULL) {
		for(i=0;i<=NB_MAX_CHARACTERS;i++) {
			if(string[i] == '\n') {
				string[i] = '\0';
				break;
			}
		}
    }
}

int ask_if_enter_again(void)
{
	bool input_start_again_ok = false;
	char char_break_or_continue[NB_MAX_CHARACTERS]="";
	while(!input_start_again_ok) {
		fprintf(stderr,"error while typing, do you want to start over ? \n(press y or n) \n");
		input(char_break_or_continue);
		if(*char_break_or_continue == 'n')
			return 0;
		else if(*char_break_or_continue == 'y')
			input_start_again_ok = true;
	}
	return 1;
}



