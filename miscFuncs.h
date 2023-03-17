#ifndef MISCFUNCS_H_SENTRY
#define MISCFUNCS_H_SENTRY


void itoa(int n, char* s);
char* concatIpAndPort(unsigned long ip, unsigned long port);
int checkClientAnswer(const char* answer);
int isCorrectLogin(const char* login);
int isCorrectPass(const char* pass);


#ifdef MISCFUNCS_SOURCE
#include "miscFuncs.c"
#endif

#endif
