#ifndef DATETIME_H_SENTRY
#define DATETIME_H_SENTRY

char* getCurTimeAsString(char*, unsigned int);
char* getCurDateAsString(char*, unsigned int);
unsigned long long getCurDateAsNumber(const char* date);

#ifdef DATETIME_SOURCE
#include "dateTime.c"
#endif

#endif
