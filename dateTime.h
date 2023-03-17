#ifndef DATETIME_H_SENTRY
#define DATETIME_H_SENTRY

unsigned int getMonthNumber(const char* month);
char* getCurTimeAsString(void);
char* getCurDateAsString(char* date);
unsigned long long getCurDateAsNumber(const char* date);
void setTimeTokens(char** time_tokens);

#ifdef DATETIME_SOURCE
#include "dateTime.c"
#endif

#endif
