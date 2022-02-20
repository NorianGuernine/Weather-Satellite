
#ifndef CONFIG_RADIO_H_
#define CONFIG_RADIO_H_

#define NB_MAX_CHARACTERS 30
#define SIZE_MSG_TO_LOG 61
#define SIZE_DATE_LOGFILE 18
#define SIZE_INFO_RADIO 95
#define QUEUE_NAME "/radio_parameters"
#define SEMAPHORE_NAME "/RTL2832U"
#define MEMORY_NAME "/process_pere"
#define MAIN_PROCESS_NAME "[main] "
#define QUEUE_PRIORITY 0644
#define SEMAPHORE_PERMISSION 0666
#define LOG_PERMISSION 0644

typedef struct info_radio info_radio;
struct info_radio {
	unsigned long freq;
	char name[NB_MAX_CHARACTERS];
	char begin_date[NB_MAX_CHARACTERS];
	char end_date[NB_MAX_CHARACTERS];
};

info_radio Lecture_infos(char *filename);
info_radio Config_manuelle(void);
int Enregistrement(void);
int logfile(char *, char *);

#endif /* CONFIG_RADIO_H_ */
