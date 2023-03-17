#ifndef LISTSTRING_H_SENTRY
#define LISTSTRING_H_SENTRY

#include "commonAPI.h"

struct listStringNode
{
	char data[21];
	struct listStringNode* nextPtr;	
};
typedef struct listStringNode* ListString;


void insertStringNode(ListString* listPtr, char* info);
char* deleteStringNode(ListString* listPtr, char* info);


#ifdef LISTSTRING_SOURCE
#include "ListString.c"
#endif


#endif
