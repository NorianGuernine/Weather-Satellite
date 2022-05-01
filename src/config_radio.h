#ifndef CONFIG_RADIO_H_
#define CONFIG_RADIO_H_

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
#include <signal.h>

#define NB_MAX_CHARACTERS 150
#define NB_CHAR_MSG_AND_NAME 181 // NB_MAX_CHARACTERS + 31 which is the number of char in the sprintf msg
#define SIZE_DATE_LOGFILE 18
#define SIZE_INFO_RADIO 95
#define QUEUE_NAME "/radio_parameters"
#define QUEUE_NB_PROCESS "/nb_process"
#define SEMAPHORE_NAME "/RTL2832U"
#define MEMORY_NAME "/process_pere"
#define MAIN_PROCESS_NAME "[main] "
#define QUEUE_PRIORITY 0644
#define SEMAPHORE_PERMISSION 0666
#define LOG_PERMISSION 0777

typedef struct info_radio info_radio;
struct info_radio {
	unsigned long freq;
	char name[NB_MAX_CHARACTERS];
	char begin_date[NB_MAX_CHARACTERS];
	char end_date[NB_MAX_CHARACTERS];
};

int read_infos(info_radio *, char *);
int manual_config(info_radio *);
int record(void);
int check_if_param(char *);
int ask_for_watchdog(void);
uint32_t set_watchdog(void);
int logfile(char *, char *);
int send_queue(mqd_t, info_radio);
uint8_t ask_for_number_sat(void);
int ask_for_date(char *);
int input(char *, FILE *);
int ask_if_enter_again(void);
void catch_child_signal(int);

#endif /* CONFIG_RADIO_H_ */
