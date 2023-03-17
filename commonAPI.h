#ifndef COMMONAPI_H_SENTRY
#define COMMONAPI_H_SENTRY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define BUFSIZE 1024
#define SERVER_CODES_COUNT 41

void clearScreen(void);
void clear_stdin(void);
void printUserRecord(char** args, unsigned int args_size);
void printDebugRecord(char** args, unsigned int args_size);

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)

#endif

#ifdef COMMONAPI_SOURCE
#include "commonAPI.c"
#endif

#endif
