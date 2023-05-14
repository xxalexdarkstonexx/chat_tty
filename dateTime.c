#ifndef DATETIME_C_SENTRY
#define DATETIME_C_SENTRY

#define _XOPEN_SOURCE
#include "dateTime.h"
#include <time.h>
#include <string.h>


char* getCurTimeAsString(char* current_time, unsigned int ct_size)
{
	if ( ct_size < 20 )
		ct_size = 20;

	time_t cur_time_in_secs = time(0);
	struct tm* cur_time = NULL;
	cur_time = localtime(&cur_time_in_secs);
	strftime( current_time, ct_size, "%H:%M:%S", cur_time );

	return current_time;
}
char* getCurDateAsString(char* date, unsigned int date_size)
{
	if (date_size < 40)
		date_size = 40;

	time_t cur_time_in_secs = time(0);
	struct tm* cur_time = NULL;
	cur_time = localtime(&cur_time_in_secs);
	strftime(date, date_size, "%a %d %b %Y %H:%M:%S", cur_time);

	return date;
}
unsigned long long getCurDateAsNumber(const char *date)
{
	struct tm time;
	memset(&time, 0, sizeof(time));
	strptime(date, "%a %d %b %Y %H:%M:%S", &time);
	
	return mktime(&time);
}

#endif
