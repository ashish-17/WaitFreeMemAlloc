#include "logging.h"
#include <time.h>
#include <pthread.h>
#include <stdarg.h>

typedef struct {
	int year;
	int mon;
	int day;
	int hour;
	int min;
	int sec;
} SystemDate;

#define MAX_MSG_SIZE 1024

void get_system_date(SystemDate *mdate)
{
	time_t rawtime;
	struct tm *timeinfo;
	rawtime = time(NULL);
	timeinfo = localtime(&rawtime);

	mdate->year = timeinfo->tm_year+1900;
	mdate->mon = timeinfo->tm_mon+1;
	mdate->day = timeinfo->tm_mday;
	mdate->hour = timeinfo->tm_hour;
	mdate->min = timeinfo->tm_min;
	mdate->sec = timeinfo->tm_sec;
}

void log_msg(char* msg, ...)
{
	char output[MAX_MSG_SIZE];
	char string[MAX_MSG_SIZE];
	SystemDate mdate;

	get_system_date(&mdate);

	va_list args;
	va_start(args, msg);
	vsprintf(string, msg, args);
	va_end(args);

	sprintf(output, "%02d.%02d.%02d-%02d:%02d:%02d - %d - %s",
			mdate.year, mdate.mon, mdate.day, mdate.hour,
			mdate.min, mdate.sec, pthread_self(), string);

	printf("\n%s", output);
}

void log_msg_epilog(char* msg, ...) {
	char string[MAX_MSG_SIZE];

	va_list args;
	va_start(args, msg);
	vsprintf(string, msg, args);
	va_end(args);

	char output[MAX_MSG_SIZE] = "EPILOG - ";
	strcat(output, string);

	log_msg(output);
}

void log_msg_prolog(char* msg, ...) {
	char string[MAX_MSG_SIZE];

	va_list args;
	va_start(args, msg);
	vsprintf(string, msg, args);
	va_end(args);

	char output[MAX_MSG_SIZE] = "PROLOG - ";
	strcat(output, string);

	log_msg(output);
}


