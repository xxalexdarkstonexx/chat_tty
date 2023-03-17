#ifndef SERVERCORE_C_SENTRY
#define SERVERCORE_C_SENTRY
#include "serverCore.h"
#include "ListString.h"
#include "commonAPI.h"
#include "serverCommands.h"
#include "serverDbConfigs.h"
#include "dateTime.h"
#include "miscFuncs.h"
#include <stdio.h>
#include <stddef.h>
#include <time.h>


#define QUEUE_SOCK_LEN 16
ListString clients_online = NULL;
Server* serv = NULL;


void session_send_string(ClientSession *sess, const char *str)
{
	int mes_size = strlen(str)+1;
	int bytes_sent = write(sess->sockfd, str, mes_size);
	if ( serv->sess_array[sess->sockfd] )
		printf("[%s] [INFO]: Sent %d bytes to %s\n", getCurTimeAsString(), bytes_sent, serv->sess_array[sess->sockfd]->lastIP);
	else
		printf("[%s] [INFO]: Sent %d bytes to %d socket\n", getCurTimeAsString(), bytes_sent, sess->sockfd);
	view_data(str, bytes_sent, 'c', 50);
	view_data(str, bytes_sent, 'd', 50);
}
static ClientSession* make_new_session(int sockfd, struct sockaddr_in *from)
{
	ClientSession* sess = malloc(sizeof(ClientSession));

	unsigned long ip = ntohl(from->sin_addr.s_addr);
	unsigned int port = ntohs(from->sin_port);
	char* bufIP = concatIpAndPort(ip, port);
	
	int j = 0;
	while ( bufIP[j] )
	{
		sess->lastIP[j] = bufIP[j];
		j++;
	}
	sess->lastIP[j] = '\0';
	free(bufIP);

	sess->sockfd = sockfd;
	sess->ID = -1;
	sess->authorized = 0;
	sess->buf_used = 0;
	sess->state = fsm_start;
	sess->user_status = status_offline;
	sess->muted = 0;
	sess->mute_time = 0;
	sess->start_mute_time = 0;
	sess->mute_time_left = 0;

	session_send_string(sess, "*CLIENT_HAS_ACCOUNT\n");

	return sess;
}
static void session_handler_has_account(ClientSession* sess, const char* client_line)
{
	if (checkClientAnswer(client_line))
	{
		session_send_string(sess, "*LOGIN_WAIT_LOGIN\n");
		sess->state = fsm_login_process_wait_login;
	}
	else
	{
		session_send_string(sess, "*SIGNUP_WAIT_LOGIN\n");
		sess->state = fsm_signup_wait_login;
	}

}
static void session_handler_login_wait_login(ClientSession* sess, const char* client_line)
{
	ListString list = clients_online;
	while (list)
	{
		if ( strcmp(client_line, list->data) == 0 )
		{
			session_send_string(sess, "*LOGIN_ALREADY_AUTHORIZED\n");
			return;
		}
		list = list->nextPtr;
	}

	int index = getDBRecordIndexByName(client_line);
	if ( (index > -1) && (strcmp(client_line, "undefined") != 0) )
	{
		int j;
		for (j = 0; client_line[j]; j++)
			sess->login[j] = client_line[j];
		sess->login[j] = '\0';

		session_send_string(sess, "*LOGIN_WAIT_PASS\n");
	 	sess->state = fsm_login_process_wait_pass;
		return;
	}
	
	session_send_string(sess, "*LOGIN_NOT_EXIST\n");
}
static void send_message_authorized(ClientSession* sess, const char* str)
{
	char message[100];
	int i;
	for (i = 0; str[i]; i++)
		message[i] = str[i];

	int j = 0;
	for (; sess->login[j]; i++, j++)
		message[i] = sess->login[j];
	message[i] = '\n';
	i++;
	message[i] = '\0';
		
	for (i = 0; i < serv->sess_array_size; i++)
		if ( ( serv->sess_array[i] ) )
		{
			if ( (i == sess->sockfd) || (!serv->sess_array[i]->authorized) )
				continue;
			write(i, message, strlen(message)+1);
		}
}
static void success_new_authorized(ClientSession* sess)
{
	char str[BUFSIZE];
	const char* code = "*SUCCESSFULLY_AUTHORIZED|";
	
	int j;
	for (j = 0; code[j]; j++)
		str[j] = code[j];

	int k = 0;
	for (; sess->login[k]; k++, j++)
		str[j] = sess->login[k];
	str[j] = '\n';
	j++;
	str[j] = '\0';

	session_send_string(sess, str);
	
	send_message_authorized(sess, "*USER_AUTHORIZED|");
}
static void session_handler_login_wait_pass(ClientSession* sess, const char* client_line)
{
	FILE* dbusers = NULL;
	FILE* dbxusers = NULL;
	
	ListString list = clients_online;
	while (list)
	{
		if ( strcmp(sess->login, list->data) == 0 )
		{
			session_send_string(sess, "*LOGIN_ALREADY_AUTHORIZED\n");
			sess->state = fsm_login_process_wait_login;
			return;
		}
		list = list->nextPtr;
	}
	
	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(), DB_USERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");		
		sess->state = fsm_error;
		return;
	}
	
	int index = getDBRecordIndexByName(sess->login);
	DBUsersInformation*	buf = malloc(sizeof(DBUsersInformation));
	fseek(dbusers, index*sizeof(DBUsersInformation), SEEK_SET);
	fread(buf, sizeof(DBUsersInformation), 1, dbusers);
	fclose(dbusers);
	
	if ( strcmp(client_line, buf->pass) == 0 )
	{
		sess->ID = buf->ID;

		int j;
		for (j = 0; client_line[j]; j++)
			sess->pass[j] = client_line[j];
		sess->pass[j] = '\0';

		if (!(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(), DB_XUSERINFO_NAME);
			session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
			sess->state = fsm_error;
			return;
		}
		DBXUsersInformation* bufXUsers = malloc(sizeof(DBXUsersInformation));
		fseek(dbxusers, sess->ID*sizeof(DBXUsersInformation), SEEK_SET);
		fread(bufXUsers, sizeof(DBXUsersInformation), 1, dbxusers);
		fclose(dbxusers);
		
		char lastIn[100];
		getCurDateAsString(lastIn);
		int i;
		for (i = 0; lastIn[i]; i++)
			sess->lastDateIn[i] = lastIn[i];
		sess->lastDateIn[i] = '\0';

		for (i = 0; bufXUsers->registrationDate[i]; i++)
			sess->registrationDate[i] = bufXUsers->registrationDate[i];
		sess->registrationDate[i] = '\0';
		
		setUserRank(sess);

		sess->start_mute_time = bufXUsers->start_mute_time;
		sess->mute_time = bufXUsers->mute_time;
		sess->muted = bufXUsers->muted;
		free(bufXUsers);

		if ( sess->muted )
			eval_mute_time_left(sess);
	
		updateDBUsersRecords(sess);
		updateDBXUsersRecords(sess);
		
		sess->authorized = 1;
		sess->user_status = status_online;
		insertStringNode(&clients_online, sess->login);
		sess->state = fsm_wait_message;
		success_new_authorized(sess);
	}
	free(buf);
	if ( !(sess->authorized) )
		session_send_string(sess, "*PASS_NOT_MATCH\n");
}
static void session_handler_signup_wait_login(ClientSession* sess, const char* client_line)
{
	if ( isCorrectLogin(client_line) )
	{
		int index = getDBRecordIndexByName(client_line);

		if (index > -1)
		{
			session_send_string(sess, "*LOGIN_ALREADY_USED\n");
			return;
		}
		else if (index < -1)
		{
			session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
			return;
		}
		
		int i;
		for (i = 0; client_line[i]; i++)
			sess->login[i] = client_line[i];
		sess->login[i] = '\0';

		session_send_string(sess, "*SIGNUP_WAIT_PASS\n");
		sess->state = fsm_signup_wait_pass;
		return;
	}

	session_send_string(sess, "*LOGIN_INCORRECT\n");
}
static void session_handler_signup_wait_pass(ClientSession* sess, const char* client_line)
{
	FILE* dbusers;


	if ( isCorrectPass(client_line) )
	{
		int records_size = 0;
		evaluate_size_db(&records_size);
		
		if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Cannot open database file \"%s\". Is it exist?\n", getCurTimeAsString(), DB_USERINFO_NAME);
			session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
			sess->state = fsm_error;
			return;
		}

		int i;
		for (i = 0 ; i < records_size; i++)
		{
			DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
			fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
			fread(buf, sizeof(DBUsersInformation), 1, dbusers);
			
			if ( strcmp(buf->username, "undefined") == 0 )
			{
				free(buf);
				break;
			}
			free(buf);
		}
		fclose(dbusers);

		int index = i;
		if (index == records_size) 
		{
			fprintf(stderr, "[%s] [ERROR]: Database file is full\n", getCurTimeAsString());
			session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
			sess->state = fsm_error;
			return;
		}
		
		sess->ID = index;

		for (i = 0; client_line[i]; i++)
			sess->pass[i] = client_line[i];
		sess->pass[i] = '\0';

		char lastIn[100];
		getCurDateAsString(lastIn);
		for (i = 0; lastIn[i]; i++)
		{
			sess->lastDateIn[i] = lastIn[i];
			sess->registrationDate[i] = lastIn[i];
		}
		sess->lastDateIn[i] = '\0';
		sess->registrationDate[i] = '\0';

		setUserRank(sess);

		sess->muted = 0;
		sess->start_mute_time = 0;
		sess->mute_time_left = 0;
		sess->mute_time = 0;

		updateDBUsersRecords(sess);
		updateDBXUsersRecords(sess);

		sess->authorized = 1;
		sess->user_status = status_online;
		insertStringNode(&clients_online, sess->login);
		sess->state = fsm_wait_message;
		success_new_authorized(sess);

		return;
	}
	session_send_string(sess, "*NEW_PASS_INCORRECT\n");
}
static void session_handler_wait_message(ClientSession* sess, const char* client_line)
{
	char** cmd_args;
	unsigned int args_num = 0;
	unsigned int cmd_num = 0;


	cmd_args = isReceivedMessageCommand(client_line, &cmd_num, &args_num);

	switch ( cmd_num )
	{
		case -2:
			command_overlimit_length_handler(sess);
			break;
		case -1:
			session_send_string(sess, "*UNKNOWN_COMMAND\n");
			break;
		case 0:
			if ( !sess->muted  )
				text_message_handler(sess, client_line, 0, NULL);
			else
			{
				make_response_to_victim(NULL, sess->login);
				if ( !sess->muted )
					text_message_handler(sess, client_line, 0, NULL);
			}
			break;
		case 1:
			help_command_handler(sess, cmd_args, args_num);
			break;
		case 2:
			whoih_command_handler(sess, cmd_args, args_num);
			break;
		case 3:
			chgpass_command_handler(sess, cmd_args, args_num);
			break;
		case 4:
			if ( sess->rank == 617 )
				op_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[4]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 5:
			if ( sess->rank == 617 )
				deop_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[5]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 6:
			if ( !sess->muted )
				pm_command_handler(sess, cmd_args, args_num);
			else
			{
				make_response_to_victim(NULL, sess->login);
				if ( !sess->muted )
				{
					pm_command_handler(sess, cmd_args, args_num);
				}
				else
				{
					if ( !clear_cmd_args(cmd_args, args_num) )
					{
						fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[6]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
						cmd_args = NULL;
					}
				}
			}
			break;
		case 7:
			status_command_handler(sess, cmd_args, args_num);
			break;
		case 8:
			record_command_handler(sess, cmd_args, args_num);
			break;
		case 9:
			if ( sess->rank == 617 )
				mute_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[9]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 10:
			if ( sess->rank == 617 )
				unmute_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[10]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 11:
			if ( sess->rank == 617 )
				kick_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[11]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 12:
			if ( sess->rank == 617 )
				table_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[12]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 13:
			if ( sess->rank == 617 )
				ban_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[13]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "*UNKNOWN_COMMAND\n");
			}
			break;
		case 14:
			if ( sess->rank == 617 )
				unban_command_handler(sess, cmd_args, args_num);
			else
			{
				if ( !clear_cmd_args(cmd_args, args_num) )
				{
					fprintf(stderr, "[%s] [WARN]: Unable to clear cmd args(in \"session_handler_wait_message\"[14]). \"cmd_args\" value is %p\n", getCurTimeAsString(), cmd_args);
					cmd_args = NULL;
				}
				session_send_string(sess, "UNKNOWN_COMMAND\n");
			}
	}
}
static void session_fsm_step(ClientSession* sess, char* client_line)
{
	switch(sess->state)
	{
		case fsm_client_has_account:
			session_handler_has_account(sess, client_line);
			free(client_line);
			break;
		case fsm_login_process_wait_login:
			session_handler_login_wait_login(sess, client_line);
			free(client_line);
			break;
		case fsm_login_process_wait_pass:
			session_handler_login_wait_pass(sess, client_line);
			free(client_line);
			break;
		case fsm_signup_wait_login:
			session_handler_signup_wait_login(sess, client_line);
			free(client_line);
			break;
		case fsm_signup_wait_pass:
			session_handler_signup_wait_pass(sess, client_line);
			free(client_line);
			break;
		case fsm_wait_message:
			session_handler_wait_message(sess, client_line);
			free(client_line);
			break;
		case fsm_finish:
		case fsm_error:
			free(client_line);
	}
}
static void session_check_lf(ClientSession* sess)
{
	int pos = -1;

	int i;
	for (i = 0; i < sess->buf_used; i++)
		if (sess->buf[i] == '\n')
		{
			pos = i;
			break;
		}
	if (pos == -1)
		return;

	char* client_line = malloc(pos+1);

	memcpy(client_line, sess->buf, pos);
	client_line[pos] = '\0';

	printf("( %s )\n", client_line);
	session_fsm_step(sess, client_line);
}
static int session_do_read(ClientSession* sess)
{
	int received = read(sess->sockfd, sess->buf, BUFSIZE);
	printf("[%s] [INFO]: Received %d bytes from %s => ", getCurTimeAsString(), received, sess->lastIP);
	
	if (received < 1)
	{
		sess->state = fsm_error;
		printf("%s\n", "()");
		return 0;
	}
	sess->buf_used += received;
	session_check_lf(sess);
	
	if (sess->buf_used >= BUFSIZE)
	{
		sess->state = fsm_error;
		return 0;
	}
	if (sess->state == fsm_finish)
		return 0;

	return 1;
}
int server_init(unsigned int port)
{
	struct sockaddr_in addr;


	printf("[%s] [INFO]: Creating socket..\n", getCurTimeAsString());
	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == -1)
	{
		fprintf(stderr, "[%s] [ERROR]: socket() failed. {%d}\n", getCurTimeAsString(), GETSOCKETERRNO());
		return 0;
	}
#ifdef _WIN32
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, "TRUE", 4);
#else
	int opt = 1; /*Prevent sticking TCP port*/
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("[%s] [INFO]: Binding socket to local address..\n", getCurTimeAsString());
	if ( bind(listen_sock, (struct sockaddr*) &addr, sizeof(addr)) < 0 )
	{
		fprintf(stderr, "[%s] [ERROR]: Failed to bind() a port. May be it has already used?\n", getCurTimeAsString());
		return 0;
	}
	printf("[%s] [INFO]: Listening..\n", getCurTimeAsString());
	if ( listen(listen_sock, QUEUE_SOCK_LEN) < 0 )
	{
		fprintf(stderr, "[%s] [ERROR]: listen() failed. {%d}\n", getCurTimeAsString(), GETSOCKETERRNO());
		return 0;
	}
	
	int records_size = 0;
	evaluate_size_db(&records_size);
	printf("[%s] [INFO]: Initialization server..\n", getCurTimeAsString());
	serv = malloc(sizeof(Server));
	serv->ls = listen_sock;
	serv->sess_array = malloc(records_size * sizeof(ClientSession*));
	serv->sess_array_size = records_size;

	int i;
	for (i = 0; i < records_size; i++)
		serv->sess_array[i] = NULL;
	
	printf("[%s] [INFO]: Initialization files..\n", getCurTimeAsString());
	initUserInfoDbFile(records_size, 0);
	initExtUserInfoDbFile(records_size, 0);
	initOpsFile(records_size);

	printf("[%s] [INFO]: Waiting for connections..\n", getCurTimeAsString());
	return 1;
}
static int server_accept_client(void)
{
	FILE* cfgPtr;
	char param_string[CONFIG_STRING_SIZE] = "currentDbSize=";
	int param_len = strlen(param_string);
	char num[10];
	int client_sock;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);


	client_sock = accept(serv->ls, (struct sockaddr*) &addr, &len);
	if (client_sock == -1)
	{
		fprintf(stderr, "[%s] [ERROR]: accept() failed. {%d}\n", getCurTimeAsString(), GETSOCKETERRNO());
		return -1;
	}

	printf("client_sock = %d\n", client_sock);
	if (client_sock >= serv->sess_array_size)
	{
		int newlen = serv->sess_array_size;
		while (client_sock >= newlen)
			newlen += serv->sess_array_size;

		serv->sess_array = realloc(serv->sess_array, newlen*sizeof(ClientSession*));
		
		int records_num = newlen - serv->sess_array_size;
		initUserInfoDbFile(records_num, 1);
		initExtUserInfoDbFile(records_num, 1);

		int i;
		for (i = serv->sess_array_size; i < newlen; i++)
			serv->sess_array[i] = NULL;
		serv->sess_array_size = newlen;
		
		itoa(newlen, num);
		int j = 0;
		for (i = param_len; num[j] != '\0'; j++, i++)
			param_string[i] = num[j];
		param_string[i] = '\0';
		
		if (!(cfgPtr = fopen(CONFIG_NAME, "r+")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to open \"%s\" config file.Trying to create one for you.\n", getCurTimeAsString(), CONFIG_NAME);
			if (!(cfgPtr = fopen(CONFIG_NAME, "w")) )
			{
				fprintf(stderr, "[%s] [ERROR]: You don't have permission to create file in this directory.\n", getCurTimeAsString());
				return -1;
			}
			fprintf(cfgPtr, "%s\n", param_string);
		}
		else
		{
			rewind(cfgPtr);
			fprintf(cfgPtr, "%s\n", param_string);
		}
		fclose(cfgPtr);
	}

	/*
	printf("%s\n", "USERSINFO");
	printDBUsers();
	printf("\n\n\n%s\n", "EXTUSERSINFO");
	printDBXUsers();
	*/

	serv->sess_array[client_sock] = make_new_session(client_sock, &addr);
	printf("[%s] [INFO]: New connection from %s\n", getCurTimeAsString(), serv->sess_array[client_sock]->lastIP);

	return 0;
}
void server_close_session(int sock_num)
{
	if ( serv->sess_array[sock_num]->muted )
		eval_mute_time_left(serv->sess_array[sock_num]);

	updateDBXUsersRecords(serv->sess_array[sock_num]);

	if ( clients_online )
		deleteStringNode(&clients_online, serv->sess_array[sock_num]->login);
	
	if ( serv->sess_array[sock_num]->authorized )
		send_message_authorized(serv->sess_array[sock_num], "*USER_LEFT_CHAT|");

	close(sock_num);
	printf("[%s] [INFO]: Lost connection from %s\n", getCurTimeAsString(), serv->sess_array[sock_num]->lastIP);

	free(serv->sess_array[sock_num]);
	serv->sess_array[sock_num] = NULL;
}
int server_running(void)
{
	fd_set readfds;
	int res, max_d;

	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(serv->ls, &readfds);
		max_d = serv->ls;

		int i;
		for (i = 0; i < serv->sess_array_size; i++)
		{
			if (serv->sess_array[i])
			{
				FD_SET(i, &readfds);
				if (i > max_d)
					max_d = i;
			}
		}

		res = select(max_d+1, &readfds, NULL, NULL, NULL);
		if (res == -1)
		{
			fprintf(stderr, "[%s] [ERROR]: select() failed. {%d}\n", getCurTimeAsString(), GETSOCKETERRNO());
			return 1;
		}

		if ( FD_ISSET(serv->ls, &readfds) )
			server_accept_client();

		for (i = 0; i < serv->sess_array_size; i++)
			if (serv->sess_array[i] && FD_ISSET(i, &readfds))
				if ( !session_do_read(serv->sess_array[i]) )
					server_close_session(i);
	}

	printf("%s\n", "Closing socket..");
	CLOSESOCKET(serv->ls);
	free(serv);

	return 0;
}

#endif
