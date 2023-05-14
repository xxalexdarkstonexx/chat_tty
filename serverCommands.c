#ifndef SERVERCOMMANDS_C_SENTRY
#define SERVERCOMMANDS_C_SENTRY

#include "ListString.h"
#include "commonAPI.h"
#include "dateTime.h"
#include "serverCore.h"
#include "serverCommands.h"
#include "serverDbConfigs.h"


enum
{
		CUR_TIME_SIZE = 100
};

extern ListString clients_online;
extern Server* serv;
extern const char* server_codes_list[SERVER_CODES_COUNT];


int clear_cmd_args(char** cmd_args, unsigned int args_num)
{
	int i;
	for (i = 0; i < args_num; i++)
		free(cmd_args[i]);
	free(cmd_args);

	if ( cmd_args == NULL )
		return 1;

	return 0;
}
static void make_send_buf(char* send_buf, int cur_pos, const char* cmd_list[])
{
	int i = 0;
	while (cmd_list[i])
	{
		int j;
		for (j = 0; cmd_list[i][j]; j++, cur_pos++)
			send_buf[cur_pos] = cmd_list[i][j];
		i++;
		if (cmd_list[i])
		{
			send_buf[cur_pos] = '|';
			cur_pos++;
		}
		else
		{
			send_buf[cur_pos] = '\n';
			cur_pos++;
			send_buf[cur_pos] = '\0';
		}
	}
}
void command_overlimit_length_handler(ClientSession *sess)
{
	char buf[100] = "*CMD_ARG_OVERLIMIT_LENGTH|";
	char max_length_arg_str[3];

	itoa(CMD_ARGS_MAX_LENGTH, max_length_arg_str, 3);
	int i, j;
	for (i = 0; buf[i]; i++) {}
	int k = 0;
	for (j = i; max_length_arg_str[k]; j++, k++)
		buf[j] = max_length_arg_str[k];
	buf[j] = '\n';
	j++;
	buf[j] = '\0';
	
	session_send_string(sess, buf);
}
void help_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	char cur_time[CUR_TIME_SIZE];
	char send_buf[BUFSIZE];
	const char* user_cmd_list[] = {
										"/help - list all valid commands",
										"/whoih - list all online users",
										"/changepassword <new_password> - set new password for account",
										"/record [<\"username\"/realname/age/quote>] [<\"string\"/\"string\"/\"integer\"/\"string\">] - show user's personal card",
										"/pm <user> <message> - send private message to user",
										"/status [status_name/list] - Show your status or set status to your record/show list valid statuses",
										NULL
								  };

	const char* admin_cmd_list[] = {
										"/help - list all valid commands",
										"/whoih - list all online users",
										"/changepassword <new_password> - set new password for account",
										"/record [<\"username\"/debug/realname/age/quote>] [<\"string\"/\"string\"/\"integer\"/\"string\">] - show user's personal card",
										"/pm <user> <message> - send private message to user",
										"/status [status_name/list] - Show your status or set status to your record/show list valid statuses",
										"/table <list/\"db_name\"> - print table with specified name",
										"/mute <user> <duration_in_seconds> - turn off possibility for user to chat",
										"/unmute <user> - turn on possibility for user to chat",
										"/ban <username/ip> <\"name\"/\"ip\"> <temp/perm> [duration_in_seconds] - block the user using \"username\" or \"ip\" rule",
										"/unban <user> - unblock user",
										"/kick <user> - force disconnect user from chat",
										"/op <user> - move user in Admin's group",
										"/deop <user> - remove user from Admin's group",
										NULL
								   };

	const char* server_cmd_list[] = {
										"/help - list all valid commands",
										"/whoih - list all online users",
										"/record <debug> <\"username\"> - show user's personal card",
										"/table <list/\"db_name\"> - print table with specified name",
										"/mute <user> <duration_in_seconds> - turn off possibility for user to chat",
										"/unmute <user> - turn on possibility for user to chat",
										"/ban <username/ip> <\"name\"/\"ip\"> <temp/perm> [duration_in_seconds] - block the user using \"username\" or \"ip\" rule",
										"/unban <user> - unblock user",
										"/kick <user> - force disconnect user from chat",
										"/op <user> - move user in Admin's group",
										"/deop <user> - remove user from Admin's group",
										"/stop - save config files, update db records and then stop server",
										NULL
									};
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"help_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}

	if (args_num > 1)
	{
		session_send_string(sess, "*COMMAND_PARAMS_NO_NEED\n");
		return;
	}
	
	const char* help = "*HELP_COMMAND_SUCCESS|";
	int k;
	for (k = 0; help[k]; k++)
		send_buf[k] = help[k];
	send_buf[k] = '\0';
	int pos = k;

	if (sess->rank != 617)
		make_send_buf(send_buf, pos, user_cmd_list);
	else
		make_send_buf(send_buf, pos, admin_cmd_list);
	
	
	session_send_string(sess, send_buf);
}
void whoih_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	char send_buf[BUFSIZE];
	char cur_time[CUR_TIME_SIZE];
	ListString clients_list = clients_online;
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"whoih_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;	
	}

	if (args_num > 1)
	{
		session_send_string(sess, "*COMMAND_PARAMS_NO_NEED\n");
		return;
	}
	
	const char* whoih = "*WHOIH_COMMAND_SUCCESS|";
	int k;
	for (k = 0; whoih[k]; k++)
		send_buf[k] = whoih[k];
	int cur_pos = k;

	while ( clients_list )
	{
		int i;
		for ( i = 0; i < serv->sess_array_size; i++ )
			if ( serv->sess_array[i] )
				if ( strcmp(clients_list->data, serv->sess_array[i]->login) == 0 )
					break;

		if (serv->sess_array[i]->user_status != 900 || sess->rank == 617)
		{
			for (k = 0; clients_list->data[k]; k++, cur_pos++)
				send_buf[cur_pos] = clients_list->data[k];
		}
		
		clients_list = clients_list->nextPtr;

		if ( clients_list )
		{
			send_buf[cur_pos] = '|';
			cur_pos++;
		}
	}
	
	send_buf[cur_pos] = '\n';
	cur_pos++;
	send_buf[cur_pos] = '\0';

	session_send_string(sess, send_buf);
}
void chgpass_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	char bufferPass[100];
	char cur_time[CUR_TIME_SIZE];

	if ( args_num != 2 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|CHGPWD|TOO_MUCH_ARGS\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"chgpass_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	int i = 0;
	for (; cmd_args[1][i]; i++)
		bufferPass[i] = cmd_args[1][i];
	bufferPass[i] = '\0';

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"chgpass_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;	
	}

	if ( isValidAuthString(bufferPass, 1) )
	{
		int i;
		for (i = 0; bufferPass[i]; i++)
			sess->pass[i] = bufferPass[i];
		sess->pass[i] = '\0';

		updateDBUsersRecords(sess);
		session_send_string(sess, "*CHGPWD_COMMAND_SUCCESS\n");
	}
	else
		session_send_string(sess, "*CHGPWD_COMMAND_INCORRECT_PASS\n");
}
void setUserRank(ClientSession *sess)
{
	int isOP = 0;
	int opsCount = 0;
	char** ops_strings;
	ops_strings = parseOpsFile(&opsCount);
	
	int j;
	for ( j = 0; j < opsCount; j++ )
	{
		if ( strcmp(sess->login, ops_strings[j]) == 0 )
			isOP = 1;
		free(ops_strings[j]);
	}
	free(ops_strings);
	ops_strings = NULL;

	if ( isOP )
	{
		sess->rank = 617;
	}
	else
	{
		unsigned long long time_user_exist = getCurDateAsNumber(sess->lastDateIn) - getCurDateAsNumber(sess->registrationDate);
		if ( time_user_exist < 7*86400 )
			sess->rank = 1;
		else if ( (time_user_exist >= 7*86400) && (time_user_exist < 30*86400) )
			sess->rank = 2;
		else if ( (time_user_exist >= 30*86400) && (time_user_exist < 365*86400) )
			sess->rank = 3;
		else
			sess->rank = 4;
	}
}
char getUserRank(ClientSession *sess)
{
	switch (sess->rank)
	{
		case 1:
			return 'F';
		case 2:
			return 'M';
		case 3:
			return 'W';
		case 4:
			return 'O';
		case 617:
			return 'A';
	}
	return 'u';
}
void op_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	FILE* dbusers;
	FILE* dbops;
	int index = -1;
	int isOnline = 0;
	char bufferUsername[100];
	char cur_time[CUR_TIME_SIZE];


	if ( args_num != 2 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|OP|TOO_MUCH_ARGS\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"op_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	int i = 0;
	for (; cmd_args[1][i]; i++)
		bufferUsername[i] = cmd_args[1][i];
	bufferUsername[i] = '\0';

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"op_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}

	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [WARN]: Can not open database file \"%s\".Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		return;
	}

	int records_size = 0;
	evaluate_size_db(&records_size);
	
	for (i = 0; i < records_size; i++)
	{
		DBUsersInformation* rbuf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(rbuf, sizeof(DBUsersInformation), 1, dbusers);

		if ( (strcmp(rbuf->username, bufferUsername) == 0) && (strcmp(bufferUsername, "undefined") != 0) )
		{
			if (rbuf->rank[0] != 'A')
			{
				ListString clientsList = clients_online;
				while (clientsList)
				{
					if (strcmp(clientsList->data, rbuf->username) == 0)
					{
						isOnline = 1;
						int j;
						for (j = 0; j < records_size; j++)
							if (serv->sess_array[j])
								if (strcmp(rbuf->username, serv->sess_array[j]->login) == 0)
								{
									index = j;
									break;
								}
						break;
					}
					clientsList = clientsList->nextPtr;
				}

				if (!isOnline)
				{
					ClientSession* buf_sess = malloc(sizeof(ClientSession));
					buf_sess->ID = rbuf->ID;
					int j;
					for (j = 0; rbuf->username[j]; j++)
						buf_sess->login[j] = rbuf->username[j];
					buf_sess->login[j] = '\0';
					for (j = 0; rbuf->pass[j]; j++)
						buf_sess->pass[j] = rbuf->pass[j];
					buf_sess->pass[j] = '\0';
					buf_sess->rank = 617;

					updateDBUsersRecords(buf_sess);
					free(buf_sess);
				}
				else
				{
					serv->sess_array[index]->rank = 617;
					updateDBUsersRecords(serv->sess_array[index]);
				}
				free(rbuf);
				break;
			}
			else
			{
				session_send_string(sess, "*OP_COMMAND_USER_ALREADY_ADMIN\n");
				fclose(dbusers);
				free(rbuf);
				return;
			}
		}
		free(rbuf);
	}
	
	if (i == records_size)
	{
		fclose(dbusers);
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|OP|USER_NOT_FOUND\n");
		return;
	}


	char** ops_strings = NULL;
	int stringsCount = 0;
	ops_strings = parseOpsFile(&stringsCount);

	if ( !(dbops = fopen("ops.txt", "w")) )
	{
		fprintf(stderr, "[%s] [WARN]: You don't have permission to rewrite \"ops.txt\" file\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		sess->state = fsm_error;
		session_send_string(sess, "*NO_PERM_TO_CREATE_FILE\n");

		int i;
		for (i = 0; i < stringsCount; i++)
			free(ops_strings[i]);

		if (stringsCount > 0)
			free(ops_strings);

		return;
	}

	if (stringsCount > 0)
		ops_strings = realloc(ops_strings, sizeof(char*)*(stringsCount+1) );
	else
		ops_strings = malloc(sizeof(char*)*(stringsCount+1));

	int str_len = 0;
	for (i = 0; bufferUsername[i]; i++)
		str_len++;

	ops_strings[stringsCount] = malloc(sizeof(char)*str_len+1);
	for (i = 0; i < str_len; i++)
		ops_strings[stringsCount][i] = bufferUsername[i];
	ops_strings[stringsCount][i] = '\0';
	stringsCount++;

	for (i = 0; i < stringsCount; i++)
	{
		fprintf(dbops, "%s\n", ops_strings[i]);
		free(ops_strings[i]);
	}
	fclose(dbops);
	free(ops_strings);

	session_send_string(sess, "*OP_COMMAND_SUCCESS\n");
}
void deop_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	FILE* dbusers;
	FILE* dbops;
	int isOnline = 0;
	int index = -1;
	ClientSession* buf_sess = NULL;
	char bufferUsername[100];
	char cur_time[CUR_TIME_SIZE];

	if (args_num != 2)
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|DEOP|TOO_MUCH_ARGS\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"deop_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	int i = 0;
	for ( ; cmd_args[1][i]; i++)
		bufferUsername[i] = cmd_args[1][i];
	bufferUsername[i] = '\0';

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"deop_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;	
	}

	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [WARN]: Can not open database file \"%s\".Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		return;
	}

	int records_size = 0;
	evaluate_size_db(&records_size);
	
	for (i = 0; i < records_size; i++)
	{
		DBUsersInformation* rbuf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(rbuf, sizeof(DBUsersInformation), 1, dbusers);

		if ( (strcmp(rbuf->username, bufferUsername) == 0) && (strcmp(bufferUsername, "undefined") != 0) )
		{
			if (rbuf->rank[0] == 'A')
			{
				ListString clientsList = clients_online;

				while (clientsList)
				{
					if (strcmp(clientsList->data, rbuf->username) == 0)
					{
						isOnline = 1;
						int j;
						for (j = 0; j < records_size; j++)
							if (serv->sess_array[j])
								if (strcmp(rbuf->username, serv->sess_array[j]->login) == 0)
								{
									index = j;
									break;
								}
						break;
					}
					clientsList = clientsList->nextPtr;
				}
				
				if (!isOnline)
				{
					buf_sess = malloc(sizeof(ClientSession));

					buf_sess->ID = rbuf->ID;
					int j;
					for (j = 0; rbuf->username[j]; j++)
						buf_sess->login[j] = rbuf->username[j];
					buf_sess->login[j] = '\0';
					for (j = 0; rbuf->pass[j]; j++)
						buf_sess->pass[j] = rbuf->pass[j];
					buf_sess->pass[j] = '\0';
					
					FILE* dbxusers;
					if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
					{
						fclose(dbusers);
						free(buf_sess);
						free(rbuf);
						fprintf(stderr, "[%s] [WARN]: Can not open database file \"%s\".Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_XUSERINFO_NAME);
						session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
						return;
					}
					DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));
					fseek(dbxusers, rbuf->ID*sizeof(DBXUsersInformation), SEEK_SET);
					fread(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
					
					for (j = 0; xbuf->registrationDate[j]; j++)
						buf_sess->registrationDate[j] = xbuf->registrationDate[j];
					buf_sess->registrationDate[j] = '\0';

					fclose(dbxusers);
					free(xbuf);
				}
				free(rbuf);
				break;
			}
			else
			{
				session_send_string(sess, "*DEOP_COMMAND_USER_ALREADY_USER\n");
				fclose(dbusers);
				free(rbuf);
				return;
			}
		}
		free(rbuf);
	}
	
	if (i == records_size)
	{
		fclose(dbusers);
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|DEOP|USER_NOT_FOUND\n");
		return;
	}


	char** ops_strings = NULL;
	int stringsCount = 0;
	ops_strings = parseOpsFile(&stringsCount);
	int isOnly = 0;

	if ( !(dbops = fopen("ops.txt", "w")) )
	{
		fprintf(stderr, "[%s] [WARN]: You don't have permission to rewrite \"ops.txt\" file\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		sess->state = fsm_error;
		session_send_string(sess, "*NO_PERM_TO_CREATE_FILE\n");

		int i;
		for (i = 0; i < stringsCount; i++)
			free(ops_strings[i]);

		if (stringsCount > 0)
			free(ops_strings);

		return;
	}
	
	if (stringsCount < 1)
	{
		fclose(dbops);
		session_send_string(sess, "*DEOP_COMMAND_USER_ALREADY_USER\n");
		return;
	}
	else
	{
		if (stringsCount > 1)
		{
			int i;
			for (i = 0; i < stringsCount; i++)
				if ( strcmp(ops_strings[i], bufferUsername) == 0 )
					break;
			int pos = i;

			int len = 0;
			for (i = 0; ops_strings[stringsCount-1][i]; i++)
				len++;

			ops_strings[pos] = realloc(ops_strings[pos], sizeof(char)*len+1);
			for (i = 0; ops_strings[stringsCount-1][i]; i++)
				ops_strings[pos][i] = ops_strings[stringsCount-1][i];
			ops_strings[pos][i] = '\0';

			free(ops_strings[stringsCount-1]);
			stringsCount--;
		}
		else
			isOnly = 1;
	}

	if (!isOnly)
	{
		for (i = 0; i < stringsCount; i++)
		{
			fprintf(dbops, "%s\n", ops_strings[i]);
			free(ops_strings[i]);
		}
	}
	else
		free(ops_strings[0]);

	free(ops_strings);
	ops_strings = NULL;
	fclose(dbops);
	
	if ( !isOnline )
	{
		setUserRank(buf_sess);
		updateDBUsersRecords(buf_sess);
		free(buf_sess);
	}
	else
	{
		setUserRank(serv->sess_array[index]);
		serv->sess_array[index]->user_status = 1;
		updateDBUsersRecords(serv->sess_array[index]);
	}	

	session_send_string(sess, "*DEOP_COMMAND_SUCCESS\n");
}
void pm_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	int userOffline = 1;
	char bufferUsername[100];
	char bufferMessage[BUFSIZE];
	char cur_time[CUR_TIME_SIZE];

	if ( args_num != 3 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|PM|TOO_MUCH_ARGS\n");
		if ( args_num > 0 )
		{
			if ( !clear_cmd_args(cmd_args, args_num) )
			{
				fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"pm_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
				cmd_args = NULL;
			}
		}
		return;
	}
	
	int i;
	for (i = 0; cmd_args[1][i]; i++)
		bufferUsername[i] = cmd_args[1][i];
	bufferUsername[i] = '\0';
	
	for (i = 0; cmd_args[2][i]; i++)
		bufferMessage[i] = cmd_args[2][i];
	bufferMessage[i] = '\0';

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"pm_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}

	if ( strcmp(bufferUsername, sess->login) == 0 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|PM|SELF_USE\n");
		return;
	}

	ListString list = clients_online;
	while (list)
	{
		if (strcmp(list->data, bufferUsername) == 0)
		{
			userOffline = 0;
			text_message_handler(sess, bufferMessage, 1, bufferUsername);
			break;
		}
		list = list->nextPtr;
	}
	
	if ( userOffline )
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|PM|USER_OFFLINE\n");
}
static char* getStatusAsString(enum status user_status)
{
	switch (user_status)
	{
		case status_offline:
			return "offline";
		case status_online:
			return "online";
		case status_busy:
			return "busy";
		case status_away:
			return "away";
		case status_work:
			return "work";
		case status_invisible:
			return "invisible";
	}
	return "undefined";
}
void status_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	const char* success = "*STATUS_COMMAND_SUCCESS|";
	char bufferMessage[BUFSIZE];
	char bufferStatus[100];
	char cur_time[CUR_TIME_SIZE];

	if (args_num > 2)
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|STATUS|TOO_MUCH_ARGS\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"status_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	if (args_num == 2)
	{
		enum status stat;
		
		int i;
		for (i = 0; cmd_args[1][i]; i++)
			bufferStatus[i] = cmd_args[1][i];
		bufferStatus[i] = '\0';

		char user_buffer[100];
		char* userStatusPtr = getStatusAsString(sess->user_status);	
		for (i = 0; userStatusPtr[i]; i++)
			user_buffer[i] = userStatusPtr[i];
		user_buffer[i] = '\0';

		if ( strcmp(user_buffer, bufferStatus) == 0)
		{
			session_send_string(sess, "*STATUS_COMMAND_ALREADY_SET\n");	
			return;
		}

		if ( strcmp(bufferStatus, "list") == 0 )
		{
			int i;
			for (i = 0; success[i]; i++)
				bufferMessage[i] = success[i];
			int pos = i;
			int j;
			for (i = 1; i <= 4; i++)
			{
				stat = i;
				char* statPtr = getStatusAsString(stat);
				for (j = 0; statPtr[j]; pos++, j++)
					bufferMessage[pos] = statPtr[j];
				bufferMessage[pos] = '|';
				pos++;
			}

			if (sess->rank == 617)
			{
				stat = 900;
				char* statPtr = getStatusAsString(stat);
				for (j = 0; statPtr[j]; pos++, j++)
					bufferMessage[pos] = statPtr[j];
				bufferMessage[pos] = '\n';
				pos++;
				bufferMessage[pos] = '\0';
			}
			else
			{
				bufferMessage[pos-1] = '\n';
				bufferMessage[pos] = '\0';
			}
			
			session_send_string(sess, bufferMessage);
		}
		else
		{
			char buffer[BUFSIZE];
			int i, j;
			for (i = 1; i <= 4; i++)
			{
				stat = i;
				char* statPtr = getStatusAsString(stat);
				for (j = 0; statPtr[j]; j++)
					buffer[j] = statPtr[j];
				buffer[j] = '\0';

				if ( strcmp(buffer, bufferStatus) == 0 )
				{
					sess->user_status = i;
					session_send_string(sess, "*STATUS_COMMAND_SUCCESS\n");
					break;
				}
			}

			if ( (i > 4) )
			{
				int flag = 0;
				if (sess->rank == 617)
				{
					stat = 900;
					char* statPtr = getStatusAsString(stat);
					for (j = 0; statPtr[j]; j++)
						buffer[j] = statPtr[j];
					buffer[j] = '\0';

					if ( strcmp(buffer, bufferStatus) == 0 )
					{
						sess->user_status = 900;
						session_send_string(sess, "*STATUS_COMMAND_SUCCESS\n");
						flag = 1;
					}
				}
				if (!flag)
					session_send_string(sess, "*STATUS_COMMAND_INCORRECT_STATUS\n");
			}
		}
	}
	else if (args_num < 2)
	{
		int i;
		for (i = 0; success[i]; i++)
			bufferMessage[i] = success[i];
		int pos = i;
		
		char* statPtr = getStatusAsString(sess->user_status);
		for (i = 0; statPtr[i]; i++)
			bufferStatus[i] = statPtr[i];
		bufferStatus[i] = '\n';
		i++;
		bufferStatus[i] = '\0';

		for (i = 0; bufferStatus[i]; pos++, i++)
			bufferMessage[pos] = bufferStatus[i];
		bufferMessage[pos] = '\0';

		session_send_string(sess, bufferMessage);
	}

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"status_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}
}
static ResponseRecord* userShowRecord(const char* registered_username, int show_record_flag)
{
	char bufferUsername[50];
	char bufferStatus[10];
	char bufferRank[10];
	char bufferRegDate[50];
	char bufferAge[4];
	char bufferRealname[50];
	char bufferQuote[70];
	FILE* dbusers;
	FILE* dbxusers;
	ResponseRecord* response_struct = NULL;
	char cur_time[CUR_TIME_SIZE];

	int records_size = 0;
	evaluate_size_db(&records_size);

	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		return NULL;
	}

	int index = -1;
	int i;
	for (i = 0; i < records_size; i++)
	{
		DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(buf, sizeof(DBUsersInformation), 1, dbusers);
		
		if ( strcmp(buf->username, registered_username) == 0 )
		{
			const char* undef = "undefined";
			const char* fresh = "FRESHMAN";
			const char* member = "MEMBER";
			const char* wisdom = "WISDOM";
			const char* old = "OLD";
			const char* admin = "ADMIN";
			
			int j;
			switch (buf->rank[0])
			{
				case 'u':
					for (j = 0; undef[j]; j++)
						bufferRank[j] = undef[j];
					bufferRank[j] = '\0';
					break;
				case 'F':
					for (j = 0; fresh[j]; j++)
						bufferRank[j] = fresh[j];
					bufferRank[j] = '\0';
					break;
				case 'M':
					for (j = 0; member[j]; j++)
						bufferRank[j] = member[j];
					bufferRank[j] = '\0';
					break;
				case 'W':
					for (j = 0; wisdom[j]; j++)
						bufferRank[j] = wisdom[j];
					bufferRank[j] ='\0';
					break;
				case 'O':
					for (j = 0; old[j]; j++)
						bufferRank[j] = old[j];
					bufferRank[j] = '\0';
					break;
				case 'A':
					for (j = 0; admin[j]; j++)
						bufferRank[j] = admin[j];
					bufferRank[j] = '\0';
			}
			
			itoa(buf->age, bufferAge, 4);
			
			j = 0;
			bufferRealname[j] = '\"';
			j++;
			int k;
			for (k = 0; buf->realname[k]; k++, j++)
				bufferRealname[j] = buf->realname[k];
			bufferRealname[j] = '\"';
			j++;
			bufferRealname[j] = '\0';
			
			for (j = 0; j < ((72-strlen(buf->quote))/2); j++)
				bufferQuote[j] = ' ';
			bufferQuote[j] = '\"';
			j++;
			for (k = 0; buf->quote[k]; k++, j++)
				bufferQuote[j] = buf->quote[k];
			bufferQuote[j] = '\"';
			j++;
			bufferQuote[j] = '\0';

			free(buf);
			fclose(dbusers);
			break;
		}
		free(buf);
	}
	index = i;

	for (i = 0; i < serv->sess_array_size; i++)
		if ( serv->sess_array[i] )
			if ( serv->sess_array[i]->authorized )
				if ( (strcmp(registered_username, serv->sess_array[i]->login) == 0) )
				{
					int j;
					const char* status = getStatusAsString(serv->sess_array[i]->user_status);
					for (j = 0; status[j]; j++)
						bufferStatus[j] = status[j];
					bufferStatus[j] = '\0';
					break;
				}

	if (i == serv->sess_array_size)
	{
		const char* offline = "offline";

		int j;
		for (j = 0; offline[j]; j++)
			bufferStatus[j] = offline[j];
		bufferStatus[j] = '\0';
	}
	
	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_XUSERINFO_NAME);
		return NULL;
	}
	
	DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));
	fseek(dbxusers, index*sizeof(DBXUsersInformation), SEEK_SET);
	fread(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
	fclose(dbxusers);

	for (i = 0; xbuf->registrationDate[i]; i++)
		bufferRegDate[i] = xbuf->registrationDate[i];
	bufferRegDate[i] = '\0';
	free(xbuf);
	
	const char* ends = "'s\"";
	bufferUsername[0] = '"';
	int j = 0;
	for (i = 1; registered_username[j]; j++, i++)
		bufferUsername[i] = registered_username[j];
	j = 0;
	for (; ends[j]; j++, i++)
		bufferUsername[i] = ends[j];
	bufferUsername[i] = '\0';
	
	response_struct = malloc(sizeof(ResponseRecord));
	
	for (i = 0; bufferAge[i]; i++)
		response_struct->age[i] = bufferAge[i];
	response_struct->age[i] = '\0';

	for (i = 0; bufferQuote[i]; i++)
		response_struct->quote[i] = bufferQuote[i];
	response_struct->quote[i] = '\0';

	for (i = 0; bufferRank[i]; i++)
		response_struct->rank[i] = bufferRank[i];
	response_struct->rank[i] = '\0';

	for (i = 0; bufferRealname[i]; i++)
		response_struct->realname[i] = bufferRealname[i];
	response_struct->realname[i] = '\0';

	for (i = 0; bufferRegDate[i]; i++)
		response_struct->regDate[i] = bufferRegDate[i];
	response_struct->regDate[i] = '\0';

	for (i = 0; bufferStatus[i]; i++)
		response_struct->status[i] = bufferStatus[i];
	response_struct->status[i] = '\0';

	for (i = 0; bufferUsername[i]; i++)
		response_struct->username[i] = bufferUsername[i];
	response_struct->username[i] = '\0';

	if (show_record_flag)
	{
		char** args = malloc(sizeof(char*) * USER_RECORD_FIELDS_NUM);
		args[0] = bufferUsername;
		args[1] = bufferStatus;
		args[2] = bufferRank;
		args[3] = bufferRealname;
		args[4] = bufferAge;
		args[5] = bufferRegDate;
		args[6] = bufferQuote;

		printRecord(args, USER_RECORD_FIELDS_NUM, 0);
		free(args);
	}

	return response_struct;
}
static char* debugShowRecord(ClientSession* sess, const char* registered_username, int show_record_flag, int* index)
{
	*index = -1;

	int i;
	for ( i = 0; i < serv->sess_array_size; i++)
		if ( serv->sess_array[i] )
			if ( serv->sess_array[i]->authorized )
				if ( strcmp(registered_username, serv->sess_array[i]->login) == 0 )
				{
					*index = i;
					break;
				}
			
	if (*index == -1)
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|RECORD|USER_NOT_FOUND\n");
		return NULL;
	}

	int len = strlen(serv->sess_array[(*index)]->login);
	char* bufferNick = malloc(sizeof(char)*len + 3);

	for (i = 0; serv->sess_array[(*index)]->login[i]; i++)
		bufferNick[i] = serv->sess_array[(*index)]->login[i];
	bufferNick[i] = '\'';
	i++;
	bufferNick[i] = 's';
	i++;
	bufferNick[i] = '\0';

	if (show_record_flag)
	{
		char bufID[10];
		char bufAuth[2];
		char bufUsed[10];
		char bufRank[10];
		char bufSock[10];
		char bufState[10];
		char bufStatus[10];
		char bufMuted[2];
		char bufMuteTime[10];
		char bufMuteTimeLeft[10];
		char bufStartMuteTime[30];
		
		itoa(serv->sess_array[(*index)]->ID, bufID, 10);
		itoa(serv->sess_array[(*index)]->authorized, bufAuth, 2);
		itoa(serv->sess_array[(*index)]->buf_used, bufUsed, 10);
		itoa(serv->sess_array[(*index)]->rank, bufRank, 10);
		itoa(serv->sess_array[(*index)]->sockfd, bufSock, 10);
		itoa(serv->sess_array[(*index)]->state, bufState, 10);
		itoa(serv->sess_array[(*index)]->user_status, bufStatus, 10);
		itoa(serv->sess_array[(*index)]->muted, bufMuted, 2);
		itoa(serv->sess_array[(*index)]->mute_time, bufMuteTime, 10);
		itoa(serv->sess_array[(*index)]->mute_time_left, bufMuteTimeLeft, 10);
		itoa(serv->sess_array[(*index)]->start_mute_time, bufStartMuteTime, 30);

		char** args = malloc(sizeof(char*) * DEBUG_RECORD_FIELDS_NUM);
		args[0] = bufferNick;
		args[1] = bufID;
		args[2] = bufAuth;
		args[3] = bufUsed;
		args[4] = serv->sess_array[(*index)]->lastDateIn;
		args[5] = serv->sess_array[(*index)]->lastIP;
		args[6] = serv->sess_array[(*index)]->registrationDate;
		args[7] = serv->sess_array[(*index)]->login;
		args[8] = serv->sess_array[(*index)]->pass;
		args[9] = bufRank;
		args[10] = bufSock;
		args[11] = bufState;
		args[12] = bufStatus;
		args[13] = bufMuted;
		args[14] = bufMuteTime;
		args[15] = bufMuteTimeLeft;
		args[16] = bufStartMuteTime;

		printRecord(args, DEBUG_RECORD_FIELDS_NUM, 1);
		free(args);
	}

	return bufferNick;
}
static void send_record_response(ClientSession* sess, const char* username, const char* type)
{
	char success_string[BUFSIZE];
	const char* success_code = "*RECORD_COMMAND_SUCCESS|";


	int i;
	for (i = 0; success_code[i]; i++)
		success_string[i] = success_code[i];

	int cur_pos = i;
	for (i = 0; type[i]; i++, cur_pos++)
		success_string[cur_pos] = type[i];
	success_string[cur_pos] = '|';
	cur_pos++;

	if ( strcmp(type, "record") == 0 )
	{
		ResponseRecord* response = userShowRecord(username, 0);
		const char* args_num = "10";
		for (i = 0; args_num[i]; i++, cur_pos++)
			success_string[cur_pos] = args_num[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		for (i = 0; response->username[i]; i++, cur_pos++)
			success_string[cur_pos] = response->username[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		for (i = 0; response->status[i]; i++, cur_pos++)
			success_string[cur_pos] = response->status[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; response->rank[i]; i++, cur_pos++)
			success_string[cur_pos] = response->rank[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; response->realname[i]; i++, cur_pos++)
			success_string[cur_pos] = response->realname[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; response->age[i]; i++, cur_pos++)
			success_string[cur_pos] = response->age[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; response->regDate[i]; i++, cur_pos++)
			success_string[cur_pos] = response->regDate[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; response->quote[i]; i++, cur_pos++)
			success_string[cur_pos] = response->quote[i];
		success_string[cur_pos] = '\n';
		cur_pos++;
		success_string[cur_pos] = '\0';
		free(response);
	}
	else if ( strcmp(type, "debug") == 0 )
	{
		const char* args_num = "20";
		for (i = 0; args_num[i]; i++, cur_pos++)
			success_string[cur_pos] = args_num[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		int index = -1;
		char* str_buf = debugShowRecord(sess, username, 0, &index);
		if ( str_buf == NULL )
			return;

		for (i = 0; str_buf[i]; i++, cur_pos++)
			success_string[cur_pos] = str_buf[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		free(str_buf);

		char bufID[10];
		itoa(serv->sess_array[index]->ID, bufID, 10);
		for (i = 0; bufID[i]; i++, cur_pos++)
			success_string[cur_pos] = bufID[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		char bufAuth[2];
		itoa(serv->sess_array[index]->authorized, bufAuth, 2);
		for (i = 0; bufAuth[i]; i++, cur_pos++)
			success_string[cur_pos] = bufAuth[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		char bufUsed[10];
		itoa(serv->sess_array[index]->buf_used, bufUsed, 10);
		for (i = 0; bufUsed[i]; i++, cur_pos++)
			success_string[cur_pos] = bufUsed[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; serv->sess_array[index]->lastDateIn[i]; i++, cur_pos++)
			success_string[cur_pos] = serv->sess_array[index]->lastDateIn[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; serv->sess_array[index]->lastIP[i]; i++, cur_pos++)
			success_string[cur_pos] = serv->sess_array[index]->lastIP[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		for (i = 0; serv->sess_array[index]->registrationDate[i]; i++, cur_pos++)
			success_string[cur_pos] = serv->sess_array[index]->registrationDate[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; serv->sess_array[index]->login[i]; i++, cur_pos++)
			success_string[cur_pos] = serv->sess_array[index]->login[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		for (i = 0; serv->sess_array[index]->pass[i]; i++, cur_pos++)
			success_string[cur_pos] = serv->sess_array[index]->pass[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		char bufRank[10];
		itoa(serv->sess_array[index]->rank, bufRank, 10);
		for (i = 0; bufRank[i]; i++, cur_pos++)
			success_string[cur_pos] = bufRank[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		char bufSock[10];
		itoa(serv->sess_array[index]->sockfd, bufSock, 10);
		for (i = 0; bufSock[i]; i++, cur_pos++)
			success_string[cur_pos] = bufSock[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		char bufState[10];
		itoa(serv->sess_array[index]->state, bufState, 10);
		for (i = 0; bufState[i]; i++, cur_pos++)
			success_string[cur_pos] = bufState[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		char bufStatus[10];
		itoa(serv->sess_array[index]->user_status, bufStatus, 10);
		for (i = 0; bufStatus[i]; i++, cur_pos++)
			success_string[cur_pos] = bufStatus[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		eval_mute_time_left(serv->sess_array[index]);
		updateDBXUsersRecords(serv->sess_array[index]);

		char bufMuted[2];
		itoa(serv->sess_array[index]->muted, bufMuted, 2);
		for (i = 0; bufMuted[i]; i++, cur_pos++)
			success_string[cur_pos] = bufMuted[i];
		success_string[cur_pos] = '|';
		cur_pos++;
		
		char bufMuteTime[10];
		itoa(serv->sess_array[index]->mute_time, bufMuteTime, 10);
		for (i = 0; bufMuteTime[i]; i++, cur_pos++)
			success_string[cur_pos] = bufMuteTime[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		char bufMuteTimeLeft[10];
		itoa(serv->sess_array[index]->mute_time_left, bufMuteTimeLeft, 10);
		for (i = 0; bufMuteTimeLeft[i]; i++, cur_pos++)
			success_string[cur_pos] = bufMuteTimeLeft[i];
		success_string[cur_pos] = '|';
		cur_pos++;

		char bufStartMuteTime[30];
		itoa(serv->sess_array[index]->start_mute_time, bufStartMuteTime, 30);
		for (i = 0; bufStartMuteTime[i]; i++, cur_pos++)
			success_string[cur_pos] = bufStartMuteTime[i];
		success_string[cur_pos] = '\n';
		cur_pos++;
		success_string[cur_pos] = '\0';
	}

	session_send_string(sess, success_string);
}
void record_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	FILE* dbusers;
	char bufferParam[100];
	char bufferParamValue[100];
	char cur_time[CUR_TIME_SIZE];

	if ( args_num > 3 )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|RECORD|TOO_MUCH_ARGS\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"record_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	if ( (args_num > 1) && (args_num <= 3) )
	{
		int i;
		for (i = 0; cmd_args[1][i]; i++)
			bufferParam[i] = cmd_args[1][i];
		bufferParam[i] = '\0';

		if ( args_num == 3 )
		{
			for (i = 0; cmd_args[2][i]; i++)
				bufferParamValue[i] = cmd_args[2][i];
			bufferParamValue[i] = '\0';
		}
	}
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"record_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}


	int records_size = 0;
	evaluate_size_db(&records_size);

	if ( args_num == 1 )
	{
		send_record_response(sess, sess->login, "record");
	}
	else if ( args_num == 2 )
	{
		if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
			return;
		}
		
		int index = getDBRecordIndexByName(bufferParam);
		if (index > -1)
		{
			if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
			{
				fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
				return;
			}
			DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
			fseek(dbusers, index*sizeof(DBUsersInformation), SEEK_SET);
			fread(buf, sizeof(DBUsersInformation), 1, dbusers);
			fclose(dbusers);

			if ( buf->rank[0] == 'A' )
				if ( sess->rank != 617 )
				{
					session_send_string(sess, "*COMMAND_NO_PERMS\n");
					free(buf);
					return;
				}
			send_record_response(sess, bufferParam, "record");
			free(buf);
		}
		else
			session_send_string(sess, "*COMMAND_INVALID_PARAMS|RECORD|USER_NOT_FOUND\n");
	}
	else if ( args_num == 3 )
	{
		const char* valid_params[] = { "realname", "age", "quote", "debug", NULL };
		int validParam = 0;
		int index = -1;
		char bufferSender[100];
					
		int j;
		for (j = 0; sess->login[j]; j++)
			bufferSender[j] = sess->login[j];
		bufferSender[j] = '\0';
		index = getDBRecordIndexByName(bufferSender);
		
		int i;
		for (i = 0; valid_params[i]; i++)
			if ( strcmp(bufferParam, valid_params[i]) == 0 )
			{
				if ( (strcmp(bufferParam, "debug") == 0) && (sess->rank != 617) )
				{
					session_send_string(sess, "*COMMAND_NO_PERMS\n");
					validParam = 1;
					break;
				}
				else if ( (strcmp(bufferParam, "debug") == 0) && (sess->rank == 617) )
				{	
					if ( getDBRecordIndexByName(bufferParamValue) != -1 )
					{
						validParam = 1;
						send_record_response(sess, bufferParamValue, "debug");
					}
					break;
				}
				
				if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
				{
					fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
					break;
				}

				DBUsersInformation* rbuf = malloc(sizeof(DBUsersInformation));
				fseek(dbusers, index*sizeof(DBUsersInformation), SEEK_SET);
				fread(rbuf, sizeof(DBUsersInformation), 1, dbusers);
				fclose(dbusers);

				if ( strcmp(bufferParam, "realname") == 0 )
				{
					const char* valid_syms = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
					unsigned int isCorrectSymbol;

					int k, j;
					for (j = 0; bufferParamValue[j]; j++)
					{
						isCorrectSymbol = 0;
						for (k = 0; valid_syms[k]; k++)
							if (bufferParamValue[j] == valid_syms[k])
							{
								isCorrectSymbol = 1;
								break;
							}
						if ( !isCorrectSymbol )
							break;
					}

					if ( isCorrectSymbol )
						if (j <= 30)
							validParam = 1;
					
					if ( validParam == 1 )
					{
						for (k = 0; bufferParamValue[k]; k++)
							rbuf->realname[k] = bufferParamValue[k];
						rbuf->realname[k] = '\0';
					}
				}
				else if ( strcmp(bufferParam, "age") == 0 )
				{
					int age = atoi(bufferParamValue);
					if (age > 0 && age < 150)
					{
						rbuf->age = age;
						validParam = 1;
					}
					else
						rbuf->age = -1;
				}
				else if ( strcmp(bufferParam, "quote") == 0 )
				{
					const char* valid_syms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
					unsigned int isCorrectSymbol;

					int k, j;
					for ( j = 0; bufferParamValue[j]; j++ )
					{
						isCorrectSymbol = 0;
						for ( k = 0; valid_syms[k]; k++ )
							if ( bufferParamValue[j] == valid_syms[k] )
							{
								isCorrectSymbol = 1;
								break;
							}
						if ( !isCorrectSymbol )
							break;
					}

					if ( isCorrectSymbol )
						if ( j <= 50 )
							validParam = 1;
					
					if ( validParam == 1 )
					{
						for ( k = 0; bufferParamValue[k]; k++ )
							rbuf->quote[k] = bufferParamValue[k];
						rbuf->quote[k] = '\0';
					}
				}

				if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb+")) )
				{
					fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
					session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
					free(rbuf);
					break;
				}

				fseek(dbusers, index*sizeof(DBUsersInformation), SEEK_SET);
				fwrite(rbuf, sizeof(DBUsersInformation), 1, dbusers);
				free(rbuf);
				fclose(dbusers);
			}

		if ( !validParam )
			session_send_string(sess, "*COMMAND_INVALID_PARAMS|RECORD|INCORRECT_STRING_VALUE\n");
	}
}
char* make_response_to_victim(char* im_buf, const char* extra)
{
	const char* victim_message = "*MUTE_COMMAND_YOU_MUTED|";
	char response_victim[100];
	char* time = NULL;

	int j;
	for ( j = 0; victim_message[j]; j++ )
		response_victim[j] = victim_message[j];
	int pos = j;
	
	for ( j = 0; j < serv->sess_array_size; j++ )
		if ( serv->sess_array[j] )
			if ( strcmp(serv->sess_array[j]->login, extra) == 0 )
				break;

	if ( im_buf == NULL )
	{
		eval_mute_time_left(serv->sess_array[j]);
		updateDBXUsersRecords(serv->sess_array[j]);
	}
	else
	{
		if ( j == serv->sess_array_size )
		{
			const char* code = "*COMMAND_INVALID_PARAMS|MUTE|USER_NOT_FOUND";
			int k;
			for ( k = 0; code[k]; k++)
				im_buf[k] = code[k];
			im_buf[k] = '\n';
			k++;
			im_buf[k] = '\0';
			return NULL;
		}
	}
	
	if ( serv->sess_array[j]->mute_time_left == 0 )
		return NULL;

	char mt[30];
	itoa(serv->sess_array[j]->mute_time_left, mt, 30);
	
	int i;
	if ( im_buf != NULL )
	{
		int mt_len = strlen(mt);
		time = malloc(sizeof(char) * mt_len + 1);
		for ( i = 0; mt[i]; i++ )
			time[i] = mt[i];
		time[i] = '\0';
	}

	for ( i = 0; mt[i]; pos++, i++ )
		response_victim[pos] = mt[i];
	response_victim[pos] = '\n';
	pos++;
	response_victim[pos] = '\0';

	session_send_string(serv->sess_array[j], response_victim);

	return time;
}
void build_inc_message_response(char* im_buf, int im_size, const char* inc_code, const char* extra)
{
	int j;
	for ( j = 0; inc_code[j]; j++ )
		im_buf[j] = inc_code[j];
	int cur_pos = j;

	if ( strcmp(inc_code, "*MUTE_COMMAND_SUCCESS|") == 0 )
	{	
		char* time = make_response_to_victim(im_buf, extra);
		for ( j = 0; extra[j]; cur_pos++, j++ )
			im_buf[cur_pos] = extra[j];
		im_buf[cur_pos] = '|';
		cur_pos++;
		for ( j = 0; time[j]; cur_pos++, j++ )
			im_buf[cur_pos] = time[j];
		free(time);
	}
	else
	{
		for ( j = 0; extra[j]; cur_pos++, j++ )
			im_buf[cur_pos] = extra[j];
	}

	im_buf[cur_pos] = '\n';
	cur_pos++;
	im_buf[cur_pos] = '\0';
}
void eval_mute_time_left(ClientSession* sess)
{
	time_t cur_time = time(0);
	int diff = cur_time - sess->start_mute_time;

	if ( diff >= sess->mute_time )
	{
		sess->muted = 0;
		sess->start_mute_time = 0;
		sess->mute_time_left = 0;
		sess->mute_time = 0;
	}
	else
	{
		sess->muted = 1;
		sess->mute_time_left = sess->mute_time-diff;
	}
}
void mute_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	ListString users_list = clients_online;
	FILE* dbxusers;
	char cur_time[CUR_TIME_SIZE];

	if ( args_num != 3 )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|TOO_MUCH_ARGS\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	char username_buf[30];
	int i;
	for ( i = 0; cmd_args[1][i]; i++ )
		username_buf[i] = cmd_args[1][i];
	username_buf[i] = '\0';

	if ( !isValidAuthString(username_buf, 0) )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|INCORRECT_USERNAME\n");

		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	int user_index = getDBRecordIndexByName(username_buf);
	if ( user_index < 0 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|USER_NOT_FOUND\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[3]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	int is_online = 0;
	while ( users_list )
	{
		if ( strcmp(username_buf, users_list->data) == 0 )
		{
			if ( strcmp(username_buf, sess->login) == 0 )
			{
				session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|SELF_USE\n");
			
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[4]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
					cmd_args = NULL;
				}
				return;
			}
			else
			{
				is_online = 1;
				break;
			}
		}
		users_list = users_list->nextPtr;
	}
	
	if ( !is_online )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|USER_OFFLINE\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[5]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
	{	
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[6]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));
	fseek(dbxusers, user_index*sizeof(DBXUsersInformation), SEEK_SET);
	fread(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
	fclose(dbxusers);
	int is_muted = xbuf->muted;
	free(xbuf);

	if ( is_muted )
	{	
		session_send_string(sess, "*MUTE_COMMAND_USER_ALREADY_MUTED\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[7]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	char time_val[30];
	for (i = 0; cmd_args[2][i]; i++)
		time_val[i] = cmd_args[2][i];
	time_val[i] = '\0';

	int time_mute = atoi(time_val);
	if ( !time_mute )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|INCORRECT_TIME_VALUE\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[8]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	if ( time_mute < 10 || time_mute > 3600 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|INCORRECT_TIME_RANGE\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[9]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	for ( i = 0; i < serv->sess_array_size; i++)
	{
		if ( serv->sess_array[i] )
			if ( strcmp(serv->sess_array[i]->login, username_buf) == 0 )
			{
				serv->sess_array[i]->muted = 1;
				serv->sess_array[i]->mute_time = time_mute;
				serv->sess_array[i]->mute_time_left = time_mute;
				time_t t = time(0);
				serv->sess_array[i]->start_mute_time = t;
				
				updateDBXUsersRecords(serv->sess_array[i]);
				break;
			}
	}
	if ( i >= serv->sess_array_size )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|MUTE|USER_OFFLINE\n");
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[10]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"mute_command_handler\"[11]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}
	
	char incorrect_message[100];
	build_inc_message_response(incorrect_message, 100, "*MUTE_COMMAND_SUCCESS|", username_buf);
	session_send_string(sess, incorrect_message);
}
void unmute_command_handler(ClientSession *sess, char **cmd_args, unsigned int args_num)
{
	FILE* dbxusers;
	char cur_time[CUR_TIME_SIZE];

	if ( args_num != 2 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|UNMUTE|TOO_MUCH_ARGS\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"unmute_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	char username_buf[30];
	int i;
	for ( i = 0; cmd_args[1][i]; i++ )
		username_buf[i] = cmd_args[1][i];
	username_buf[i] = '\0';
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"unmute_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}

	if ( !isValidAuthString(username_buf, 0) )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|UNMUTE|INCORRECT_USERNAME\n");
		return;
	}
	
	int user_index = getDBRecordIndexByName(username_buf);
	if ( user_index < 0 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|UNMUTE|USER_NOT_FOUND\n");
		return;
	}
	
	if ( strcmp(sess->login, username_buf) == 0 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|UNMUTE|SELF_USE\n");
		return;
	}
	
	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
	{	
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		return;
	}

	DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));
	fseek(dbxusers, user_index*sizeof(DBXUsersInformation), SEEK_SET);
	fread(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
	fclose(dbxusers);
	int is_muted = xbuf->muted;
	free(xbuf);

	if ( !is_muted )
	{	
		session_send_string(sess, "*UNMUTE_COMMAND_USER_NOT_MUTED\n");
		return;
	}
	
	for ( i = 0; i < serv->sess_array_size; i++ )
		if ( serv->sess_array[i] )
			if ( strcmp(serv->sess_array[i]->login, username_buf) == 0 )
			{
				serv->sess_array[i]->muted = 0;
				serv->sess_array[i]->mute_time = 0;
				serv->sess_array[i]->mute_time_left = 0;
				serv->sess_array[i]->start_mute_time = 0;
				updateDBXUsersRecords(serv->sess_array[i]);

				char incorrect_message[100];
				build_inc_message_response(incorrect_message, 100, "*UNMUTE_COMMAND_YOU_UNMUTED|", username_buf);
				session_send_string(serv->sess_array[i], incorrect_message);
				break;
			}

	char incorrect_message[100];
	build_inc_message_response(incorrect_message, 100, "*UNMUTE_COMMAND_SUCCESS|", username_buf);
	session_send_string(sess, incorrect_message);
}
void kick_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num)
{
	ListString users_list = clients_online;
	char cur_time[CUR_TIME_SIZE];	


	if ( args_num != 2 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|KICK|TOO_MUCH_ARGS\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"kick_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}

	char username_buf[30];
	int i;
	for ( i = 0; cmd_args[1][i]; i++ )
		username_buf[i] = cmd_args[1][i];
	username_buf[i] = '\0';
	
	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"kick_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}
	
	if ( !isValidAuthString(username_buf, 0) )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|KICK|INCORRECT_USERNAME\n");
		return;
	}
	
	int user_index = getDBRecordIndexByName(username_buf);
	if ( user_index < 0 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|KICK|USER_NOT_FOUND\n");
		return;
	}
	
	int is_online = 0;
	while ( users_list )
	{
		if ( strcmp(username_buf, users_list->data) == 0 )
		{
			if ( strcmp(username_buf, sess->login) == 0 )
			{
				session_send_string(sess, "*COMMAND_INVALID_PARAMS|KICK|SELF_USE\n");
				return;
			}
			else
			{
				is_online = 1;
				break;
			}
		}
		users_list = users_list->nextPtr;
	}
	
	if ( !is_online )
	{	
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|KICK|USER_OFFLINE\n");
		return;
	}
	
	for ( i = 0; i < serv->sess_array_size; i++ )
		if ( serv->sess_array[i] )
			if ( strcmp(serv->sess_array[i]->login, username_buf) == 0 )
			{
				session_send_string(serv->sess_array[i], "*KICK_COMMAND_SUCCESS|VICTIM\n");
				int sock = serv->sess_array[i]->sockfd;
				server_close_session(sock);
			}
	
	const char* str = "*KICK_COMMAND_SUCCESS|SENDER|";
	char response[100];
	build_inc_message_response(response, 100, str, username_buf);
	session_send_string(sess, response);
}
void table_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num)
{	
	int isUserdata = 0;
	int isSessionUserInfo = 0;
	char cur_time[CUR_TIME_SIZE];

	if ( args_num != 2 )
	{
		session_send_string(sess, "*COMMAND_INVALID_PARAMS|TABLE|TOO_MUCH_ARGS\n");
		
		if ( !clear_cmd_args(cmd_args, args_num) )
		{
			fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"table_command_handler\"[1]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
			cmd_args = NULL;
		}
		return;
	}
	
	char bufferValue[100];
	int i;
	for ( i = 0; cmd_args[1][i]; i++ )
		bufferValue[i] = cmd_args[1][i];
	bufferValue[i] = '\0';

	if ( !clear_cmd_args(cmd_args, args_num) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"table_command_handler\"[2]). \"cmd_args\" value is %p\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), cmd_args);
		cmd_args = NULL;
	}

	if ( strcmp(bufferValue, "list") == 0 )
	{
		session_send_string(sess, "*TABLE_COMMAND_SUCCESS|LIST|usersdata.dat|users_sessions_info.dat\n");
		return;
	}

	(strcmp(bufferValue, DB_USERINFO_NAME) == 0) ? (isUserdata = 1) : isUserdata;
	(strcmp(bufferValue, DB_XUSERINFO_NAME) == 0) ? (isSessionUserInfo = 1) : isSessionUserInfo;
	
	if ( isUserdata || isSessionUserInfo )
	{
		FILE* dbusers;
		FILE* dbxusers;

		const char* resp_start = "*TABLE_COMMAND_SUCCESS|DATA|";
		char response_buffer[BUFSIZE];
		int i;
		for ( i = 0; resp_start[i]; i++ )
			response_buffer[i] = resp_start[i];
		int cur_pos = i;
		
		char rec_size[10];
		int records_size = 0;
		evaluate_size_db(&records_size);
		itoa(records_size, rec_size, 10);
		for ( i = 0; rec_size[i]; cur_pos++, i++ )
			response_buffer[cur_pos] = rec_size[i];
		response_buffer[cur_pos] = '|';
		cur_pos++;
		
		if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
		{	
			session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
			return;
		}
		
		int* db_indexes = malloc(sizeof(int) * records_size);
		for ( i = 0; i < records_size; i++ )
			db_indexes[i] = -1;

		char non_empty_rec[10];
		int non_empty_recs_counter = 0;
		for (i = 0; i < records_size; i++ )
		{
			DBUsersInformation* bufUsers = malloc(sizeof(DBUsersInformation));
			fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
			fread(bufUsers, sizeof(DBUsersInformation), 1, dbusers);

			if ( (bufUsers->ID == -1) || (strcmp(bufUsers->pass, "undefined") == 0) || (strcmp(bufUsers->username, "undefined") == 0) || (bufUsers->rank[0] == 'u') )
			{
				free(bufUsers);
				continue;
			}
			db_indexes[i] = i;
			non_empty_recs_counter++;
			free(bufUsers);
		}
		fclose(dbusers);

		itoa(non_empty_recs_counter, non_empty_rec, 10);
		for ( i = 0; non_empty_rec[i]; cur_pos++, i++ )
			response_buffer[cur_pos] = non_empty_rec[i];
		response_buffer[cur_pos] = '|';
		cur_pos++;
		
		if ( isUserdata )
		{
			if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
			{	
				free(db_indexes);
				db_indexes = NULL;
				session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
				return;
			}	

			const char* dbu = "USERINFO|";
			for ( i = 0; dbu[i]; cur_pos++, i++ )
				response_buffer[cur_pos] = dbu[i];

			for ( i = 0; i < records_size; i++ )
			{
				DBUsersInformation* bufUsers = malloc(sizeof(DBUsersInformation));
				fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
				fread(bufUsers, sizeof(DBUsersInformation), 1, dbusers);

				if ( db_indexes[i] > -1 )
				{
					char bufID[10];
					itoa(bufUsers->ID, bufID, 10);
					int j;
					for (j = 0; bufID[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufID[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;

					for (j = 0; bufUsers->username[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufUsers->username[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;

					for (j = 0; bufUsers->pass[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufUsers->pass[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;

					response_buffer[cur_pos] = bufUsers->rank[0];
					cur_pos++;
					response_buffer[cur_pos] = '|';
					cur_pos++;
				}
				free(bufUsers);
			}
		}
		else
		{
			if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
			{	
				free(db_indexes);
				db_indexes = NULL;
				session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
				return;
			}

			const char* dbx = "XUSERINFO|";
			for ( i = 0; dbx[i]; cur_pos++, i++ )
				response_buffer[cur_pos] = dbx[i];

			for ( i = 0; i < records_size; i++ )
			{
				DBXUsersInformation* bufXUsers = malloc(sizeof(DBXUsersInformation));
				fseek(dbxusers, i*sizeof(DBXUsersInformation), SEEK_SET);
				fread(bufXUsers, sizeof(DBXUsersInformation), 1, dbxusers);

				if ( db_indexes[i] > -1 )
				{
					char bufID[10];
					itoa(bufXUsers->ID, bufID, 10);
					int j;
					for (j = 0; bufID[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufID[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;

					for (j = 0; bufXUsers->registrationDate[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufXUsers->registrationDate[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;

					for (j = 0; bufXUsers->lastDateIn[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufXUsers->lastDateIn[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;
					
					for (j = 0; bufXUsers->lastDateOut[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufXUsers->lastDateOut[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;
					
					for (j = 0; bufXUsers->lastIP[j]; cur_pos++, j++)
						response_buffer[cur_pos] = bufXUsers->lastIP[j];
					response_buffer[cur_pos] = '|';
					cur_pos++;
				}
				free(bufXUsers);
			}
		}
		response_buffer[cur_pos-1] = '\n';
		response_buffer[cur_pos] = '\0';
		
		free(db_indexes);
		db_indexes = NULL;

		session_send_string(sess, response_buffer);
		return;
	}
	
	session_send_string(sess, "*COMMAND_INVALID_PARAMS|TABLE|INCORRECT_STRING_VALUE\n");
}
void ban_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num)
{

}
void unban_command_handler(ClientSession* sess, char** cmd_args, unsigned int args_num)
{

}
static void drawLine(int line_length, unsigned int c)
{
	const int offset = 18;

	if ( c == 0 )
		printf("%c", '|');
	else
		printf("%s", "\n|");

	int i;
	for (i = 1; i <= (offset + 3*line_length + line_length-1); i++ )
		printf("%c", '-');
	if ( c == 0 )
		printf("%c", '|');
	else
		printf("%s", "|\n");
}
void view_data(const char* str, int str_size, char mode, int line_length )
{	
	char cur_time[CUR_TIME_SIZE];

	if ( line_length >= str_size)
		line_length = str_size-1;
	
	drawLine(line_length, 0);

	int k;
	for ( k = 0; k < str_size; k++ )
	{
		if ( (k % line_length) == 0 )
			printf("\n[%s] [INFO]: ", getCurTimeAsString(cur_time, CUR_TIME_SIZE));

		if ( mode == 'c' )
		{
			if (str[k] == '\n')
				continue;
			printf("%3c ", str[k]);
		}
		else if ( mode == 'd' )
			printf("%3d ", str[k]);
		else
		{
			printf("%s\n", "Incorrect \"mode\" value");
			return;
		}
	}
	
	drawLine(line_length, 1);
}
void text_message_handler(ClientSession *sess, const char *msg, unsigned int isPrivate, const char *adresat)
{
	char str[BUFSIZE];
	char buf[BUFSIZE];
	char cur_time[CUR_TIME_SIZE];

	int i;
	for (i = 0; msg[i]; i++)
	{
		if (msg[i] == '|')
		{
			buf[i] = ' ';
			continue;
		}
		buf[i] = msg[i];
	}
	buf[i] = '\0';
	int buf_len = strlen(buf);

	int cur_pos;
	if ( sess->rank == 617 )
	{
		const char* prefix = "[ADMIN]";
		for (i = 0; prefix[i]; i++)
			str[i] = prefix[i];
	}
	else
	{
		i = 0;
		str[i] = ' ';
		i++;
	}
	str[i] = '|';
	i++;
	cur_pos = i;

	if (isPrivate)
	{
		str[cur_pos] = '~';
		cur_pos++;
	}

	int j;
	for ( j = 0; sess->login[j]; cur_pos++, j++ )
		str[cur_pos] = sess->login[j];
	str[cur_pos] = '|';
	cur_pos++;
	

	for ( i = 0; i < SERVER_CODES_COUNT; i++ )
	{
		char* sub_str = strstr(buf, server_codes_list[i]);
		if ( sub_str )
		{
			int sub_str_len = strlen(sub_str);
			int idx = buf_len - sub_str_len;
			int code_len = strlen(server_codes_list[i]);
			
			int k;
			for ( k = idx; k < (idx+code_len); k++ )
				buf[k] = ' ';
			i--;
		}
	}
	int k = 0;
	char* buffer_str = malloc(sizeof(char) * BUFSIZE);
	for ( j = 0; buf[j]; j++, k++ )
	{
		if ( buf[j] == ' ' )
		{
			buffer_str[k] = ' ';
			for (; buf[j] == ' '; j++ ) { }
			if ( buf[j] != ' ' ) j--;
		}
		else
			buffer_str[k] = buf[j];
	}
	buffer_str[k] = '\0';

	for ( j = 0; buffer_str[j]; j++ )
		buf[j] = buffer_str[j];
	buf[j] = '\0';
	free(buffer_str);
	buf_len = strlen(buf);
	

	for (j = 0; buf[j]; j++, cur_pos++)
		str[cur_pos] = buf[j];
	str[cur_pos] = '\n';
	cur_pos++;
	str[cur_pos] = '\0';
	cur_pos++;

	int mes_size = strlen(str)+1;
	for ( i = 0; i < serv->sess_array_size; i++ )
	{
		if ( (serv->sess_array[i]) )
		{
			if ( (i == sess->sockfd) || (!serv->sess_array[i]->authorized) )
				continue;
			if ( isPrivate )
			{
				if ( strcmp(serv->sess_array[i]->login, adresat) != 0 )
					continue;

				int bytes_sent = write(i, str, mes_size); 
				printf("[%s] [INFO]: Sent %d bytes to %s\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), bytes_sent, serv->sess_array[i]->lastIP);
				view_data(str, bytes_sent, 'c', 50);
				view_data(str, bytes_sent, 'd', 50);
				break;
			}
			int bytes_sent = write(i, str, mes_size); 
			printf("[%s] [INFO]: Sent %d bytes to %s\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), bytes_sent, serv->sess_array[i]->lastIP);
			view_data(str, bytes_sent, 'c', 50);
			view_data(str, bytes_sent, 'd', 50);
		}
	}
}
char** isReceivedMessageCommand(const char* msg, unsigned int* cmd_num, unsigned int* args_num)
{
	char buf[BUFSIZE];
	char aux_buf[BUFSIZE];
	char** cmd_args;
	*args_num = 0;
	
	unsigned int k;
	for (k = 0; msg[k]; k++)
	{
		buf[k] = msg[k];
		aux_buf[k] = buf[k];
	}
	buf[k] = '\0';
	aux_buf[k] = '\0';

	if (buf[0] == '/')
	{
		char* istr = strtok(aux_buf, " ");
		while (istr)
		{
			(*args_num)++;
			istr = strtok(NULL, " ");
		}

		cmd_args = malloc(sizeof(char*) * (*args_num));
		int k = 0;
		int i;
		istr = strtok(buf, " ");
		while (istr)
		{
			int arg_length = 0;
			for (i = 0; istr[i]; i++)
				arg_length++;

			if (arg_length > CMD_ARGS_MAX_LENGTH)
			{
				free(cmd_args);
				*cmd_num = -2;
				*args_num = 0;
				return NULL;
			}

			cmd_args[k] = malloc(sizeof(char)*arg_length + 1);
			for (i = 0; istr[i]; i++)
				cmd_args[k][i] = istr[i];
			cmd_args[k][i] = '\0';

			k++;
			istr = strtok(NULL, " ");
		}

		if ( strcmp("/help", cmd_args[0]) == 0 )
		{
			*cmd_num = 1;
			return cmd_args;
		}
		if ( strcmp("/whoih", cmd_args[0]) == 0 )
		{
			*cmd_num = 2;
			return cmd_args;
		}
		if ( strcmp("/changepassword", cmd_args[0]) == 0 )
		{
			*cmd_num = 3;
			return cmd_args;
		}
		if ( strcmp("/op", cmd_args[0]) == 0 )
		{
			*cmd_num = 4;
			return cmd_args;
		}
		if ( strcmp("/deop", cmd_args[0]) == 0 )
		{
			*cmd_num = 5;
			return cmd_args;
		}
		if ( strcmp("/pm", cmd_args[0]) == 0 )
		{
			*cmd_num = 6;

			if (*args_num > 3)
			{
				char bufferCommand[100];
				char bufferName[100];
				char bufferMessage[BUFSIZE];
				
				int i;
				for (i = 0; cmd_args[0][i]; i++)
					bufferCommand[i] = cmd_args[0][i];
				bufferCommand[i] = '\0';
				for (i = 0; cmd_args[1][i]; i++)
					bufferName[i] = cmd_args[1][i];
				bufferName[i] = '\0';


				int k = 0;
				int j;
				for (i = 2; i < *args_num; i++)
				{
					for (j = 0; cmd_args[i][j]; j++)
					{
						bufferMessage[k] = cmd_args[i][j];
						k++;
					}
					bufferMessage[k] = ' ';
					k++;
				}
				bufferMessage[k-1] = '\0';

				for (i = 0; i < *args_num; i++)
					free(cmd_args[i]);
				free(cmd_args);
				

				*args_num = 3;
				cmd_args = malloc(sizeof(char*) * (*args_num));
				
				int str_len = strlen(bufferCommand);
				cmd_args[0] = malloc(sizeof(char)*str_len + 1);
				for (i = 0; i < str_len; i++)
					cmd_args[0][i] = bufferCommand[i];
				cmd_args[0][i] = '\0';

				str_len = strlen(bufferName);
				cmd_args[1] = malloc(sizeof(char)*str_len + 1);
				for (i = 0; i < str_len; i++)
					cmd_args[1][i] = bufferName[i];
				cmd_args[1][i] = '\0';

				str_len = strlen(bufferMessage);
				cmd_args[2] = malloc(sizeof(char)*str_len + 1);
				for (i = 0; i < str_len; i++)
					cmd_args[2][i] = bufferMessage[i];
				cmd_args[2][i] = '\0';
			}
			return cmd_args;
		}
		if ( strcmp("/status", cmd_args[0]) == 0 )
		{
			*cmd_num = 7;
			return cmd_args;
		}
		if ( strcmp("/record", cmd_args[0]) == 0 )
		{
			*cmd_num = 8;
			return cmd_args;
		}
		if ( strcmp("/mute", cmd_args[0]) == 0 )
		{
			*cmd_num = 9;
			return cmd_args;
		}
		if ( strcmp("/unmute", cmd_args[0]) == 0 )
		{
			*cmd_num = 10;
			return cmd_args;
		}
		if ( strcmp("/kick", cmd_args[0]) == 0 )
		{
			*cmd_num = 11;
			return cmd_args;
		}
		if ( strcmp("/table", cmd_args[0]) == 0 )
		{
			*cmd_num = 12;
			return cmd_args;
		}
		if ( strcmp("/ban", cmd_args[0]) == 0 )
		{
			*cmd_num = 13;
			return cmd_args;
		}
		if ( strcmp("/unban", cmd_args[0]) == 0 )
		{
			*cmd_num = 14;
			return cmd_args;
		}
	
		for (i = 0; i < *args_num; i++)
			free(cmd_args[i]);
		free(cmd_args);
		*cmd_num = -1;
		*args_num = 0;
		return NULL;
	}
	*cmd_num = 0;
	*args_num = 0;
	return NULL;
}


#endif
