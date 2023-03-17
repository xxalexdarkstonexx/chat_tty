#ifndef SERVERCOMMANDS_H_SENTRY
#define SERVERCOMMANDS_H_SENTRY

#include "serverCore.h"

#define CMD_ARGS_MAX_LENGTH 200

typedef struct
{
	char username[50];
	char status[10];
	char rank[10];
	char regDate[50];
	char age[4];
	char realname[50];
	char quote[70];
} ResponseRecord;

void setUserRank(ClientSession *sess);
char getUserRank(ClientSession *sess);
void eval_mute_time_left(ClientSession *sess);
char* make_response_to_victim(char* im_buf, const char* extra);
void build_inc_message_response(char* im_buf, int im_size, const char* inc_code, const char* extra);
int clear_cmd_args(char** cmd_args, unsigned int args_num);
void view_data(const char* str, int str_size, char mode, int line_length);
void command_overlimit_length_handler(ClientSession* sess);
void help_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num);
void whoih_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num);
void chgpass_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num);
void op_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num);
void deop_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num);
void pm_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void status_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void record_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void mute_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void unmute_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void kick_command_handler(ClientSession* sess, char **cmd_args, unsigned int args_num);
void table_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num);
void ban_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num);
void unban_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num);
void text_message_handler(ClientSession *sess, const char *msg, unsigned int isPrivate, const char* adresat);
char** isReceivedMessageCommand(const char* msg, unsigned int* cmd_num, unsigned int* args_num);


#ifdef SERVERCOMMANDS_SOURCE
#include "serverCommands.c"
#endif

#endif
