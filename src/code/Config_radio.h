
#ifndef CONFIG_RADIO_H_
#define CONFIG_RADIO_H_

#define NB_MAX_CHARACTERS 30
#define SIZE_INFO_RADIO 95
#define QUEUE_NAME "/radio_parameters"
#define SEMAPHORE_NAME "/RTL2832U"
#define QUEUE_PRIORITY 0644
#define SEMAPHORE_PERMISSION 0666

typedef struct info_radio info_radio;
struct info_radio {	//Structure stockant les informations radios personnelles à chaque thread
	unsigned long freq;
	char name[NB_MAX_CHARACTERS];
	char begin_date[NB_MAX_CHARACTERS];
	char end_date[NB_MAX_CHARACTERS];
};

info_radio Lecture_infos(char *filename);	//Fonction qui lit les paramètres enregistrés dans des fichiers
info_radio Config_manuelle(void);

#endif /* CONFIG_RADIO_H_ */
