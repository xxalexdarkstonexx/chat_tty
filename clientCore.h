#ifndef CLIENTCORE_H_SENTRY
#define CLIENTCORE_H_SENTRY

/*
 *	Следующие константы используются для настройки
 *	чувствительности антиспам модуля, используемого
 *	в файле tcp_client2.c
 * */

enum
{
	ANTISPAM_MODULE_MSG_CNT = 5,
	ANTISPAM_MODULE_TOTAL_TIME_MS = 5000,
	ANTISPAM_MODULE_MESSAGES_INTERVAL = 300
};

int checkServerResponse(char** response_tokens, unsigned int response_tokens_size, int* authorized);


#ifdef CLIENTCORE_SOURCE
#include "clientCore.c"
#endif

#endif
