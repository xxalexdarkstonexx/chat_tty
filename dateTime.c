#ifndef DATETIME_C_SENTRY
#define DATETIME_C_SENTRY

#include "dateTime.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* getCurTimeAsString(void)
{
	char* time_tokens[5];
	time_t current_time = time(0);
	char* buf = ctime(&current_time);

	char* istr = strtok(buf, " ");
	int k = 0;
	while (istr)
	{
		time_tokens[k] = istr;
		k++;
		istr = strtok(NULL, " ");
	}

	return time_tokens[3];
}
char* getCurDateAsString(char* date)
{
	char* time_tokens[5];
	time_t current_time = time(0);
	char* buf = ctime(&current_time);

	char* istr = strtok(buf, " ");
	int k = 0;
	while (istr)
	{
		time_tokens[k] = istr;
		k++;
		istr = strtok(NULL, " ");
	}

	for (k = 0; time_tokens[0][k]; k++)
		date[k] = time_tokens[0][k];
	date[k] = ' ';
	k++;
	int cur_pos = k;
	
	for (k = 0; time_tokens[2][k]; k++, cur_pos++)
		date[cur_pos] = time_tokens[2][k];
	date[cur_pos] = ' ';
	cur_pos++;

	for (k = 0; time_tokens[1][k]; k++, cur_pos++)
		date[cur_pos] = time_tokens[1][k];
	date[cur_pos] = ' ';
	cur_pos++;

	for (k = 0; time_tokens[4][k] != '\n'; k++, cur_pos++)
		date[cur_pos] = time_tokens[4][k];
	date[cur_pos] = ' ';
	cur_pos++;

	for (k = 0; time_tokens[3][k]; k++, cur_pos++)
		date[cur_pos] = time_tokens[3][k];
	date[cur_pos] = '\0';

	return date;
}
static unsigned int getExtraYears(unsigned int year, unsigned int *flag)
{
	unsigned int startExtraYear = 1972;
	unsigned int result = 0;

	int i;
	for (i = startExtraYear; i <= year; i += 4)
	{
		result++;
		if (i == year)
			*flag = 1;
	}
	return result;
}
unsigned int getMonthNumber(const char* month)
{
	const char* months[] = {
									"Jan",
									"Feb",
									"Mar",
									"Apr",
									"May",
									"Jun",
									"Jul",
									"Aug",
									"Sep",
									"Oct",
									"Nov",
									"Dec",
									NULL
						   };

	unsigned int i = 0;
	for ( ; months[i]; i++ )
		if (strcmp(month, months[i]) == 0)
			return (i+1);

	return 0;
}
unsigned long long getCurDateAsNumber(const char *date)
{
	const int gtm_3 = 10800;
	const int normal_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	const int extra_days[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned int date_size = strlen(date);
	char buf[date_size];
	unsigned int flagValue = 0;
	unsigned int* flag = &flagValue;
	unsigned long long result = 0;

	int i;
	for (i = 0; i < date_size; i++)
		buf[i] = date[i];
	buf[i] = '\0';

	char* time_tokens[5];
	char* istr = strtok(buf, " ");
	i = 0;
	while (istr)
	{
		time_tokens[i] = istr;
		i++;
		istr = strtok(NULL, " ");
	}

	unsigned int year  = atoi(time_tokens[3]);
	unsigned int month = getMonthNumber(time_tokens[2]);
	unsigned int day = atoi(time_tokens[1]);

	unsigned int extraYears = getExtraYears(year, flag);
	unsigned int normYears = year-1970-extraYears;
	unsigned long long value = normYears*365*86400;

	result += value;
	value = extraYears*366*86400;
	result += value;
	
	if (*flag)
	{
		int i;
		for ( i = 0; i < (month-1); i++ )
			result += (extra_days[i]*86400);
	}
	else
	{
		int i;
		for (i = 0; i < (month-1); i++)
			result += (normal_days[i]*86400);
	}
	result += day*86400;

	char buf_time[100];
	for (i = 0; time_tokens[4][i]; i++)
		buf_time[i] = time_tokens[4][i];
	buf_time[i] = '\0';

	char* hhmmss[3];
	istr = strtok(buf_time, ":");
	i = 0;
	while (istr)
	{
		hhmmss[i] = istr;
		i++;
		istr = strtok(NULL, ":");
	}
	unsigned int hours, minutes, seconds;
	hours = atoi(hhmmss[0]);
	minutes = atoi(hhmmss[1]);
	seconds = atoi(hhmmss[2]);

	result += (hours*3600 + minutes*60 + seconds);

	return result-(gtm_3+86400);
}
void setTimeTokens(char** time_tokens)
{
	time_t current_time = time(0);
	char* buf = ctime(&current_time);
	char* istr = strtok(buf, " ");

	int k = 0;
	while ( istr )
	{
		time_tokens[k] = istr;
		k++;
		istr = strtok(NULL, " ");
	}
}

#endif
