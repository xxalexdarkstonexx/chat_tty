#ifndef CLIENTCORE_H_SENTRY
#define CLIENTCORE_H_SENTRY

#define AS_MSG_CNT 5
#define AS_TOTAL_TIME_MS 5000
#define AS_MESSAGE_INTERVAL 300

void show_logo(void);
int get_string(char* buf, unsigned int length, unsigned int* chars_num);
int sendall(int s, const char* buf, int* buf_size);
void printHorizontalLine(unsigned int offset, unsigned int string_len, unsigned char type);
void printTextFrame(const char* str[]);
int restrictMessageLength(char* read);
void deleteExtraSpaces(char* read);
char* getCode(void);
int checkServerResponse(char** response_tokens, unsigned int response_tokens_size, int peer_sock, int* authorized);
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define TIMER_START() (GetTickCount())
int inet_ws_aton(const char* cp, struct in_addr* inp);
#else
unsigned long long get_tick_unix(void);
#define TIMER_START() (get_tick_unix())
#endif

#ifdef CLIENTCORE_SOURCE
#include "clientCore.c"
#endif

#endif
