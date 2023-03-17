#ifndef SERVERDBCONFIGS_H_SENTRY
#define SERVERDBCONFIGS_H_SENTRY

#include "serverCore.h"

#define CONFIG_NAME "server.properties"
#define CONFIG_STRINGS_NUM 10
#define CONFIG_STRING_SIZE 80
#define CONFIG_SETTING_DEFAULT_SIZE 10

char* getLogFilename(char* filename_buf);
void evaluate_size_db(int* records_size);
void initUserInfoDbFile(int records_size, int extension);
void initExtUserInfoDbFile(int records_size, int extension);
void initOpsFile(int records_size);
int getDBRecordIndexByName(const char* nickname);
char** parseOpsFile(int* stringsCount);
void printDBUsers(void);
void printDBXUsers(void);
void updateDBUsersRecords(ClientSession *sess);
void updateDBXUsersRecords(ClientSession *sess);

#ifdef SERVERDBCONFIGS_SOURCE
#include "serverDbConfigs.c"
#endif

#endif
