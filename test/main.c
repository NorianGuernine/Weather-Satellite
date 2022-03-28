#include <fcntl.h>
#include "unity.h"
#include "unity_internals.h"
#include "../src/Config_radio.h"

void create_file(char *,char *);
void setup_test_read_infos(void);
void tear_down_read_infos(void);
void test_read_infos(void);
void remove_file(char *file_name);
void test_input(void);
void test_logfile(void);
void remove_file(char *);
void setup_input(void);
void tear_down_input(void);

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_input);
    RUN_TEST(test_logfile);
    RUN_TEST(test_read_infos);
    return UNITY_END();
}

void setUp()
{

}

void tearDown(void)
{

}

void test_read_infos(void)
{
	setup_test_read_infos();
	info_radio empty_file, good_file;

	/* Testing empty file */
	TEST_ASSERT_EQUAL_INT(-1,read_infos(&empty_file,"empty.txt"));

	/* Testing non complete file */
	TEST_ASSERT_EQUAL_INT(-1,read_infos(&empty_file,"only_name.txt"));
	TEST_ASSERT_EQUAL_INT(-1,read_infos(&empty_file,"name_frequency.txt"));
	TEST_ASSERT_EQUAL_INT(-1,read_infos(&empty_file,"name_freq_start_date.txt"));

	/* Testing good file */
	TEST_ASSERT_EQUAL_INT(0,read_infos(&good_file,"good_file.txt"));
	TEST_ASSERT_EQUAL_UINT64(137912500,good_file.freq);
	TEST_ASSERT_EQUAL_STRING("good_file", good_file.name);
	TEST_ASSERT_EQUAL_STRING("01-02-10-11", good_file.begin_date);
	TEST_ASSERT_EQUAL_STRING("01-02-11-12", good_file.end_date);
	tear_down_read_infos();
}

void test_input(void)
{
	setup_input();
	char string[NB_MAX_CHARACTERS] = {0};
	FILE *fp_ok, *fp_ok2, *fp_empty= NULL;
	fp_ok = fopen("input_ok.txt","r");
	fp_ok2 = fopen("input_ok2.txt","r");
	fp_empty = fopen("input_empty.txt","r");

	/* Testing file ok with  \n */
	TEST_ASSERT_EQUAL_INT(0, input(string,fp_ok));
	TEST_ASSERT_EQUAL_STRING("Test input",string);

	/* Testing file ok without \n */
	TEST_ASSERT_EQUAL_INT(0, input(string,fp_ok2));
	TEST_ASSERT_EQUAL_STRING("Test input without enter",string);

	/* Testing input with file returning NULL */
	TEST_ASSERT_EQUAL_INT(-1, input(string,fp_empty));
	tear_down_input();

}

void setup_input(void)
{
	create_file("input_ok.txt", "Test input\n");
	create_file("input_ok2.txt", "Test input without enter");
	create_file("input_empty.txt", "");
}

void reset_logfile(void)
{
	remove_file("Weather_sat.log");
}

void test_logfile(void)
{
	char string[NB_MAX_CHARACTERS] = {0};
	char date_logfile[SIZE_DATE_LOGFILE] = {0};
	char expected_msg[NB_MAX_CHARACTERS] = {0};
	FILE *fp = NULL;
	time_t t = time(NULL);
	struct tm tm;

	reset_logfile();
	/* Test creation of file */
	TEST_ASSERT_EQUAL_INT(0, logfile("[Test] ","test"));

	/* Test of content of Weather_sat.log */
	fp = fopen("Weather_sat.log","r");
	input(string,fp);
	/* Read the date to compare it later in the string assert */
	tm = *localtime(&t);

	sprintf(date_logfile,"[%02d:%02d:%02d:%02d] ",tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
	strcpy(expected_msg,date_logfile);
	strcat(expected_msg,"[Test] test");

	TEST_ASSERT_EQUAL_STRING(expected_msg,string);

	reset_logfile();
}


void tear_down_input(void)
{
	remove_file("input_ok.txt");
	remove_file("input_ok2.txt");
	remove_file("input_empty.txt");
}

void setup_test_read_infos(void)
{
	create_file("empty.txt","");
	create_file("only_name.txt","name_only");
	create_file("name_frequency.txt","frequency\n137912500");
	create_file("name_freq_start_date.txt","start_date\n137912500\n01-02-10-11");
	create_file("good_file.txt","good_file\n137912500\n01-02-10-11\n01-02-11-12");
}

void tear_down_read_infos(void)
{
	remove_file("empty.txt");
	remove_file("only_name.txt");
	remove_file("name_frequency.txt");
	remove_file("name_freq_start_date.txt");
	remove_file("good_file.txt");
}

void remove_file(char *file_name)
{
	int file_status;
	file_status = remove(file_name);
	if(file_status > 0)
		perror("remove");
}

void create_file(char *file_name, char *text)
{
	int file,size_text = 0;

	file = open(file_name, O_CREAT | O_WRONLY, 0777 );

	if(file < 0) {
		fprintf(stderr,"Error while opening file \n");
		exit(EXIT_FAILURE);
	}

	if(text == NULL) {
		fprintf(stderr,"text in create file return NULL \n");
		exit(EXIT_FAILURE);
	}
	size_text = strlen(text);
	write(file,text,size_text*sizeof(char));
	close(file);
}
