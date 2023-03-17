#ifndef MISCFUNCS_C_SENTRY
#define MISCFUNCS_C_SENTRY
#define MIN_LOGIN_LENGTH 3
#define MAX_LOGIN_LENGTH 16
#define MIN_PASS_LENGTH 4
#define MAX_PASS_LENGTH 20

#include "miscFuncs.h"
#include <string.h>
#include <stdlib.h>

static void reverse(char* s)
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
void itoa(int n, char* s)
{
	int i, sign;

	if ( (sign = n) < 0 )
		n = -n;
	i = 0;
	
	do
	{
		s[i] = n % 10 + '0';
		i++;
	}
	while ( (n /= 10) > 0 );

	if (sign < 0)
	{
		s[i] = '-';
		i++;
	}
	s[i] = '\0';
	reverse(s);
}
char* concatIpAndPort(unsigned long ip, unsigned long port)
{
	char* str = malloc(22);
	int i = 0;
	int k;
	int len = i;
	int shift = 24;

	while (shift >= 0)
	{
		k = 0;
		char buf[4];
		int ip_buf = (ip >> shift & 0xFF);
		shift -= 8;
		itoa(ip_buf, buf);
		for (; i < len+strlen(buf); i++)
		{
			str[i] = buf[k];
			k++;
		}
		str[i] = '.';
		i++;
		len = i;
	}
	str[len-1] = ':';
	char port_buf[6];
	itoa(port, port_buf);

	k = 0;
	for (; i < len+strlen(port_buf); i++)
	{
		str[i] = port_buf[k];
		k++;
	}
	str[i] = '\0';

	return str;
}
int checkClientAnswer(const char *answer)
{
	const char* checkFor[] = {
									"y",
									"Y",
									"ye",
									"yE",
									"Ye",
									"YE",
									"yes",
									"Yes",
									"yEs",
									"yeS",
									"YEs",
									"YeS",
									"yES",
									"YES",
									NULL
						     };

	int i;
	for (i = 0; checkFor[i]; i++)
		if (strcmp(answer, checkFor[i]) == 0)
			return 1;
	return 0;
}
int isCorrectLogin(const char *login)
{
	const char valid_symbols[] = 
									"abcdefghij"
									"klmnopqrst"
									"uvwxyzABCD"
									"EFGHIJKLMN"
									"OPQRSTUVWX"
									"YZ01234567"
									"89_"
								;

	int i, j;
	int correctSymbols = 0;
	for (i = 0; login[i]; i++) {}
	int login_size = i;

	if (login_size < MIN_LOGIN_LENGTH || login_size > MAX_LOGIN_LENGTH)
		return 0;

	if (strcmp(login, "undefined") == 0)
		return 0;

	for (i = 0; i < login_size; i++)
		for (j = 0; valid_symbols[j]; j++)
			if (login[i] == valid_symbols[j])
			{
				correctSymbols++;
				break;
			}
	if (correctSymbols == login_size)
		return 1;

	return 0;
}
int isCorrectPass(const char *pass)
{
	const char valid_symbols[] = 
									"abcdefghij"
									"klmnopqrst"
									"uvwxyzABCD"
									"EFGHIJKLMN"
									"OPQRSTUVWX"
									"YZ01234567"
									"89!?_$#"
								;

	unsigned int correctSymbols = 0;
	int i;
	for (i = 0; pass[i]; i++) {}
	int pass_size = i;

	if (pass_size < MIN_PASS_LENGTH || pass_size > MAX_PASS_LENGTH)
		return 0;

	if (strcmp(pass, "undefined") == 0)
		return 0;

	int j;
	for (i = 0; i < pass_size; i++)
		for (j = 0; valid_symbols[j]; j++)
			if (pass[i] == valid_symbols[j])
			{
				correctSymbols++;
				break;
			}
	if (correctSymbols == pass_size)
		return 1;

	return 0;
}


#endif
