#ifndef COMMONAPI_H_SENTRY
#define COMMONAPI_H_SENTRY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum
{
	MAX_MESSAGE_LENGTH = 200,
	BUFSIZE = 1024,
	SERVER_CODES_COUNT = 41,
	DEBUG_RECORD_FIELDS_NUM = 17,
	USER_RECORD_FIELDS_NUM = 7,
	MIN_LOGIN_LENGTH = 3,
	MAX_LOGIN_LENGTH = 16,
	MIN_PASS_LENGTH = 4,
	MAX_PASS_LENGTH = 20
};

int get_string(char* target_buf, unsigned int max_str_length, unsigned int* input_chars_num);
int sendall(int s, const char* buf, int* buf_size);
int restrictMessageLength(char* read);
void deleteExtraSpaces(char* read, int read_size);
char* getCode(void);
void clearScreen(void);
void clear_stdin(void);
void printRecord(char** args, unsigned int args_size, int debug_mode);
void itoa(int number, char* num_buf, int max_buf_len);
char* concatIpAndPort(unsigned long ip, unsigned long port);
int checkClientAnswer(const char* answer);

#ifdef _WIN32

#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define TIMER_START() (GetTickCount())
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
int inet_ws_aton(const char* cp, struct in_addr* inp);

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#define TIMER_START() (get_tick_unix())
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
unsigned long long get_tick_unix(void);

#endif

#ifdef COMMONAPI_SOURCE
#include "commonAPI.c"
#endif

#endif
