#ifndef LISTSTRING_C_SENTRY
#define LISTSTRING_C_SENTRY

#include "ListString.h"

#ifdef PRINTSTR
static void printString(ListString listPtr)
{
	if (listPtr == NULL)
		printf("%s\n", "List is empty.");
	else
	{
		printf("%s\n", "List is:");
		while (listPtr != NULL)
		{
			printf("%s --> ", listPtr->data);
			listPtr = listPtr->nextPtr;
		}
		printf("%s\n", "NULL");
	}
}
#endif
void insertStringNode(ListString* listPtr, char* info)
{
	ListString previousPtr;
	ListString currentPtr;
	ListString newPtr;

	newPtr = (ListString) malloc(sizeof(struct listStringNode));

	int i;
	for (i = 0; i < strlen(info); i++)
		newPtr->data[i] = info[i];
	newPtr->data[i] = '\0';
	newPtr->nextPtr = NULL;

	previousPtr = NULL;
	currentPtr = *listPtr;

	while (currentPtr != NULL)
	{
		previousPtr = currentPtr;
		currentPtr = currentPtr->nextPtr;
	}

	if (previousPtr == NULL)
		*listPtr = newPtr;
	else
	{
		previousPtr->nextPtr = newPtr;
		newPtr->nextPtr = currentPtr;
	}
}
char* deleteStringNode(ListString* listPtr, char* info)
{
	ListString tempPtr;
	ListString previousPtr;
	ListString currentPtr;

	if (strcmp(info, (*listPtr)->data) == 0)
	{
		tempPtr = *listPtr;
		*listPtr = (*listPtr)->nextPtr;
		free(tempPtr);
		return info;
	}
	else
	{
		previousPtr = *listPtr;
		currentPtr = (*listPtr)->nextPtr;

		while ( currentPtr != NULL && (strcmp(currentPtr->data, info) != 0) )
		{
			previousPtr = currentPtr;
			currentPtr = currentPtr->nextPtr;
		}

		if (currentPtr != NULL)
		{
			tempPtr = currentPtr;
			previousPtr->nextPtr = currentPtr->nextPtr;
			free(tempPtr);
			return info;
		}
	}
	return 0;
}


#endif
