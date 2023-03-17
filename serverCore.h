#ifndef SERVERAPI_H_SENTRY
#define SERVERAPI_H_SENTRY

#define DB_USERINFO_NAME "usersdata.dat"
#define DB_XUSERINFO_NAME "users_sessions_info.dat"

#include "commonAPI.h"

enum fsm_states
{
	fsm_start,
	fsm_client_has_account = fsm_start,
	fsm_login_process_wait_login,
	fsm_login_process_wait_pass,
	fsm_signup_wait_login,
	fsm_signup_wait_pass,
	fsm_wait_message,
	fsm_finish,
	fsm_error
};

enum status 
{
	status_offline,
	status_online,
	status_busy,
	status_away,
	status_work,
	status_invisible = 900
};

typedef struct
{
	int ID;
	int sockfd;
	char buf[BUFSIZE];
	int buf_used;
	enum fsm_states state;
	char login[18];
	char pass[22];
	char lastIP[22];
	char lastDateIn[50];
	char registrationDate[50];
	int rank;
	int authorized;
	enum status user_status;
	int muted;
	unsigned int start_mute_time;
	unsigned int mute_time;
	unsigned int mute_time_left;
} ClientSession;

typedef struct
{
	int ls;
	ClientSession** sess_array;
	unsigned long sess_array_size;
} Server;

typedef struct
{
	int ID;
	char username[18];
	char pass[22];
	char rank[2];
	char realname[50];
	int age;
	char quote[100];
} DBUsersInformation;

typedef struct
{
	int ID;
	int muted;
	unsigned int start_mute_time;
	unsigned int mute_time;
	unsigned int mute_time_left;
	char lastIP[22];
	char lastDateIn[50];
	char lastDateOut[50];
	char registrationDate[50];
} DBXUsersInformation;

void session_send_string(ClientSession *sess, const char *str); 
int server_init(unsigned int port);
void server_close_session(int sock_num);
int server_running(void);                    

#ifdef SERVERCORE_SOURCE
#include "serverCore.c"
#endif


#endif
