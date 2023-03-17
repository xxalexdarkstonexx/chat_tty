#ifndef CLIENTCORE_C_SENTRY
#define CLIENTCORE_C_SENTRY

#include "clientCore.h"
#include "commonAPI.h"
#include <bits/time.h>
#include <stddef.h>
#include <stdio.h>

#define MAX_MESSAGE_LENGTH 201
#define MAX_STRING_LENGTH 52

void show_logo(void)
{
	const char chat_logo[1024] =  
							"   $$$$$$$$\\  $$$$$$\\  $$$$$$$\\        $$$$$$\\  $$\\   $$\\  $$$$$$\\ $$$$$$$$\\\n"
							"   \\__$$  __|$$  __$$\\ $$  __$$\\      $$  __$$\\ $$ |  $$ |$$  __$$\\\\__$$  __|\n"
							"      $$ |   $$ /  \\__|$$ |  $$ |     $$ /  \\__|$$ |  $$ |$$ /  $$ |  $$ |   \n"
							"      $$ |   $$ |      $$$$$$$  |     $$ |      $$$$$$$$ |$$$$$$$$ |  $$ |    \n"
							"      $$ |   $$ |      $$  ____/      $$ |      $$  __$$ |$$  __$$ |  $$ |    \n"
							"      $$ |   $$ |  $$\\ $$ |           $$ |  $$\\ $$ |  $$ |$$ |  $$ |  $$ |    \n"
							"      $$ |   \\$$$$$$  |$$ |           \\$$$$$$  |$$ |  $$ |$$ |  $$ |  $$ |    \n"
							"      \\__|    \\______/ \\__|            \\______/ \\__|  \\__|\\__|  \\__|  \\__|\n"
						 ;	
	printf("%s", chat_logo);
}
int get_string(char *buf, unsigned int length, unsigned int* chars_num)
{
	int ch;
	int i = 0;
	*chars_num = 0;

	do
	{
		ch = getchar();
		if ( (ch != '\n') && (ch != EOF) )
		{
			if (i < length)
			{
				buf[i] = ch;
				i++;
			}
			(*chars_num)++;
		}
	}
	while ( (ch != '\n') && (ch != EOF));

	if (ch == EOF)
	{
		clear_stdin();
		fflush(stdout);
	}

	buf[i] = '\n';
	i++;
	buf[i] = '\0';
	i++;

	return i;
}
int sendall(int s, const char* buf, int* buf_size)
{
	int total = 0;
	int bytesleft = *buf_size;
	int n;

	while (total < *buf_size)
	{
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1) break;
		total += n;
		bytesleft -= n;
	}
	*buf_size = total;

	return n == -1 ? -1 : 0;
}
void printHorizontalLine(unsigned int offset, unsigned int string_len, unsigned char type)
{
	int i;
	for (i = 1; i <= offset; i++)
		printf("%c", ' ');
	for (i = 1; i <= string_len; i++)
		printf("%c", type);
    for (i = 1; i <= offset; i++)
		printf("%c", ' ');
}
void printTextFrame(const char* list_str[])
{
	int i = 0;
	int j;
	printf("%s", "\n\n");
	printHorizontalLine(0, 14, ' ');
	printf("%s\n", "Welcome to authorization page of my simple TCP chat.");
	printHorizontalLine(14, MAX_STRING_LENGTH, '#');
	printf("%c", '\n');
	while (list_str[i] != NULL)
	{
		int len = strlen(list_str[i]);
		printHorizontalLine(0, 14, ' ');
		printHorizontalLine(0, 2, '#');
		int new_len = (MAX_STRING_LENGTH-len-4)/2;
		((new_len % 2) == 1) ? new_len++ : new_len; 
		for (j = 1; j <= new_len; j++)
			printf("%c", ' ');
		printf("%s", list_str[i]);
		for (j = 1; j <= (MAX_STRING_LENGTH-len-4)/2; j++)
			printf("%c", ' ');
		printHorizontalLine(0, 2, '#');
		printf("%c", '\n');
		i++;
	}
	printHorizontalLine(14, MAX_STRING_LENGTH, '#');
	printf("%c", '\n');
	printHorizontalLine(0, 14, ' ');
	printf("%s", "Your answer: ");
}
char* getCode(void)
{
	const char symbols[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int z;
	char* buf = malloc(5);

	for (z = 0; z < 4; z++)
		buf[z] = symbols[rand() % 62];
	buf[z] = '\0';

	return buf;
}
int restrictMessageLength(char* read)
{
	int length = strlen(read);
	
	if (length > MAX_MESSAGE_LENGTH)
	{
		read[MAX_MESSAGE_LENGTH] = '\0';
		return MAX_MESSAGE_LENGTH;
	}
	return length;
}
void deleteExtraSpaces(char* read)
{
	//Delete multiple spaces from start of the message
	///////////////////////////////////////////////////////////
	int i = 0, c = 0;
	while (read[i++] == ' ') c++;
	for (i = c; i < strlen(read)+1; i++) read[i-c] = read[i];
	///////////////////////////////////////////////////////////
	
	char* message_tokens[100];
	char* istr = strtok(read, " ");
			
	int m = 0;
	i = 0;
	c = 0;
			
	while (istr != NULL)
	{
		message_tokens[i] = istr;
		while (message_tokens[i][m++] == ' ') c++;
		for (m = c; m < strlen(message_tokens[i])+1; m++) message_tokens[i][m-c] = message_tokens[i][m];
		
		i++;
		m = 0;
		c = 0;
		istr = strtok(NULL, " ");	
	}
	int size = i;	
				
	int l = 0;
	m = 0;
	for (i = 0; i < size; i++)
	{
		for (m = 0; m < strlen(message_tokens[i]); m++)
			read[l++] = message_tokens[i][m];
		read[l++] = ' ';
	}
	read[l-1] = '\0';
}
static void sendAnswer(const char** strings, char* answer, unsigned int read_chars, int peer_sock)
{
	clearScreen();
	show_logo();
	printTextFrame(strings);
	unsigned int chars_num = 0;
	int size;

	do
	{
		size = get_string(answer, read_chars, &chars_num);
	}
	while ( answer[0] == '\n' );

	char* message = malloc(size);
	int i;
	for ( i = 0; i < size-1; i++ )
		message[i] = answer[i];
	message[i] = '\0';

	if ( chars_num > read_chars )
		clear_stdin();

	sendall(peer_sock, message, &size);
	printf("Sent %d bytes\n", size);

	free(message);
}
int checkServerResponse(char **response_tokens, unsigned int response_tokens_size, int peer_sock, int* authorized)
{
	if ( strcmp(response_tokens[0], "*CLIENT_HAS_ACCOUNT") == 0 )
	{
		char answer[5];
		unsigned int read_chars = 3;
		const char* strings[] = { "Have you already have an account?", "Enter \"y\"or\"n\"", NULL };

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_WAIT_LOGIN") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = { "Enter nickname for your account", NULL };

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_ALREADY_AUTHORIZED") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = { "This account is already authorized", "Try to use another login", NULL };

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_ALREADY_USED") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = {"This login is already exist in database", "Try another one",  NULL };

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_INCORRECT") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = {"Incorrect login!", "Please, check it and try again", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_NOT_EXIST") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = {"This login doesn't exist", "Check your input string", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*SIGNUP_WAIT_LOGIN") == 0 )
	{
		char answer[18];
		unsigned int read_chars = 16;
		const char* strings[] = {"Enter nickname to create", "new account", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_WAIT_PASS") == 0 )
	{
		char answer[22];
		unsigned int read_chars = 20;
		const char* strings[] = {"Enter password for your account", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*NEW_PASS_INCORRECT") == 0 )
	{
		char answer[22];
		unsigned int read_chars = 20;
		const char* strings[] = {"Password incorrect!", "Check it and try again", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*PASS_NOT_MATCH") == 0 )
	{
		char answer[22];
		unsigned int read_chars = 20;
		const char* strings[] = {"Password doesn't match with this account", "Try again", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*SIGNUP_WAIT_PASS") == 0 )
	{
		char answer[22];
		unsigned int read_chars = 20;
		const char* strings[] = {"Enter pass for your new account", NULL};

		sendAnswer(strings, answer, read_chars, peer_sock);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*USER_AUTHORIZED") == 0 )
	{
		int i;
		printf("%s", "\n");
		for ( i = 1; i <= 28; i++ )
			printf("%c", ' ');
		printf("\"%s\" joined to GLOBAL chat\n", response_tokens[1]);

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*USER_LEFT_CHAT") == 0 )
	{
		int i;
		printf("%s", "\n");
		for ( i = 1; i <= 28; i++ )
			printf("%c", ' ');
		printf("\"%s\" left GLOBAL chat\n", response_tokens[1]);

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*CANNOT_CONNECT_DATABASE") == 0 )
	{
		clearScreen();
		fprintf(stderr, "Server cannot connect to database file. Try later\n");

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*CMD_ARG_OVERLIMIT_LENGTH") == 0 )
	{
		fprintf(stderr, "Some command or argument in your input is too long! Max length is %s\n", response_tokens[1]);

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*HELP_COMMAND_SUCCESS") == 0 )
	{
		int k;
		printf("%s\n", "--- List of all valid commands ---");
		for ( k = 1; k < response_tokens_size; k++)
			printf("[%d]: %s\n", k, response_tokens[k]);
		printf("%s\n", "----------------------------------");

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*WHOIH_COMMAND_SUCCESS") == 0 )
	{
		int usersCounter = response_tokens_size-1;

		if ( usersCounter >= 1 )
		{
			int i;
			printf("%s", "There ");
			(usersCounter > 1) ? printf("are %d users online:\n", usersCounter) : printf("is %d user online:\n", usersCounter);

			for (i = 1; i <= usersCounter; i++)
				printf("%s\n", response_tokens[i]);
			printf("%s\n", "------------------------");
		}
		else
			printf("%s\n", "There is nobody online :(");

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*CHGPWD_COMMAND_SUCCESS") == 0 )
	{
		printf("%s\n", "Your password has been successfully changed");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*CHGPWD_COMMAND_INCORRECT_PASS") == 0 )
	{
		printf("%s\n",
			   "Incorrect password. Try to follow next rules:\n"
			   "Password must be not less than 4 symbols\n"
			   "Password must be not more than 20 symbols\n"
			   "Password has to consist correct symbols(a-zA-Z0-9 and special chars)"
			  );

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*DEOP_COMMAND_SUCCESS") == 0 )
	{
		printf("%s\n", "User has been removed from Admin group");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*DEOP_COMMAND_USER_ALREADY_USER") == 0 )
	{
		printf("%s\n", "This user is not an administrator\n"
				       "or has been removed from admin's group earlier");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*OP_COMMAND_SUCCESS") == 0 )
	{
		printf("%s\n", "User has been added to Admin's group");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*OP_COMMAND_USER_ALREADY_ADMIN") == 0 )
	{
		printf("%s\n", "User is already in Admin's group");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*STATUS_COMMAND_INCORRECT_STATUS") == 0 )
	{
		printf("%s\n", "Incorrect status! Type \"/status list\" to see list of valid statuses");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*STATUS_COMMAND_ALREADY_SET") == 0 )
	{
		printf("%s\n", "You have already set this status!");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*STATUS_COMMAND_SUCCESS") == 0 )
	{
		if ( response_tokens_size == 1 )
			printf("%s\n", "Your status has been successfully changed");
		else if ( response_tokens_size == 2 )
			printf("Your current status: %s\n", response_tokens[1]);
		else if ( response_tokens_size > 2 )
		{
			printf("%s\n", "List of all valid statuses:");
			int i;
			for ( i = 1; i < response_tokens_size; i++ )
				printf("- %s\n", response_tokens[i]);
			printf("%s\n", "----------------------");
		}

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*RECORD_COMMAND_SUCCESS") == 0 )
	{
		if ( strcmp(response_tokens[1], "debug") == 0 )
		{
			char** args = malloc(sizeof(char*) * 17);
			int i, j, k;

			for ( i = 0, j = 3; i < 17; i++, j++ )
			{
				args[i] = malloc(sizeof(char) * strlen(response_tokens[j]) + 1 );
				for ( k = 0; response_tokens[j][k]; k++ )
					args[i][k] = response_tokens[j][k];
				args[i][k] = '\0';
			}
			printDebugRecord(args, 17);
			
			for ( i = 0; i < 17; i++ )
				free(args[i]);
			free(args);
		}
		else if ( strcmp(response_tokens[1], "record") == 0 )
		{
			char** args = malloc(sizeof(char*) * 7);
			int i, j, k;
			for ( i = 0, j = 3; i < 7; i++, j++)
			{
				args[i] = malloc(sizeof(char) * strlen(response_tokens[j]) + 1);
				for ( k = 0; response_tokens[j][k]; k++ )
					args[i][k] = response_tokens[j][k];
				args[i][k] = '\0';
			}
			printUserRecord(args, 7);

			for ( i = 0; i < 7; i++ )
				free(args[i]);
			free(args);
		}

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*MUTE_COMMAND_USER_ALREADY_MUTED") == 0 )
	{
		printf("%s\n", "Unable to execute a command. User has already muted!");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*MUTE_COMMAND_SUCCESS") == 0 )
	{
		printf("User \"%s\" has been muted for %s seconds.\n", response_tokens[1], response_tokens[2]);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*MUTE_COMMAND_YOU_MUTED") == 0 )
	{
		printf("You have been muted for %s seconds.\n", response_tokens[1]);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*UNMUTE_COMMAND_USER_NOT_MUTED") == 0 )
	{
		printf("%s\n", "Unable to execute a command. User is already unmuted!");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*UNMUTE_COMMAND_SUCCESS") == 0 )
	{
		printf("User \"%s\" has been successfully unmuted.\n", response_tokens[1]);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*UNMUTE_COMMAND_YOU_UNMUTED") == 0 )
	{
		printf("%s\n", "You have been unmuted.");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*KICK_COMMAND_SUCCESS") == 0 )
	{
		if ( strcmp(response_tokens[1], "SENDER") == 0 )
		{
			printf("User \"%s\" has been kicked from the chat.\n", response_tokens[2]);
			return 1;
		}
		if ( strcmp(response_tokens[1], "VICTIM") == 0 )
		{
			clearScreen();
			printf("%s\n", "You have been kicked from the chat.");
			return 1;
		}
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*TABLE_COMMAND_SUCCESS") == 0 )
	{
		if ( strcmp(response_tokens[1], "LIST") == 0 )
		{
			printf("%s\n", "List of available tables:");
			int i;
			for ( i = 2; i < response_tokens_size; i++ )
				printf("[%d]: %s\n", i-1, response_tokens[i]);
			printf("%s\n", "-------------------------");
			return 1;
		}
		
		if ( strcmp(response_tokens[1], "DATA") == 0 )
		{
			int non_empty_records_size = atoi(response_tokens[3]);
			
			if ( strcmp(response_tokens[4], "USERINFO") == 0 )
			{
				printf("\n%s\n", "File \"usersdata.dat\":");
				printf("%s\n", "------------------------------------------------------");
				printf("| %-4s | %-16s | %-20s | %-1s |\n", "ID", "Username", "Password", "R");
				printf("%s\n", "------------------------------------------------------");
				
				int i;
				int offset = 0;
				int iterations = 0;
				for ( i = 5; i < (5*non_empty_records_size+1); i++ )
				{
					if ( i == (5 + offset) )
					{
						printf("| %-4s |", response_tokens[5+offset]);
					}
					else if ( i == (5 + offset) + 1 )
					{
						printf(" %-16s |", response_tokens[5+offset+1]);
					}
					else if ( i == (5 + offset) + 2 )
					{
						printf(" %-20s |", response_tokens[5+offset+2]);
					}
					else if ( i == (5 + offset) + 3 )
					{
						printf(" %-1s |\n", response_tokens[5+offset+3]);
					}

					iterations++;
					if ( (iterations % 4) == 0 )
					{
						offset += 4;
						printf("%s\n", "------------------------------------------------------");
					}
				}
			}
			else if ( strcmp(response_tokens[4], "XUSERINFO") == 0 )
			{
				printf("\n%s\n", "File \"users_sessions_info.dat\":");
				printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
				printf("| %-4s | %-25s | %-25s | %-25s | %-25s |\n", "ID", "Reg. Date", "Last In Date", "Last Out Date", "Last IP");
				printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
				
				int i;
				int offset = 0;
				int iterations = 0;
				for ( i = 5; i < (5*non_empty_records_size+5); i++ )
				{
					if ( i == (5 + offset) )
					{
						printf("| %-4s |", response_tokens[5+offset]);
					}
					else if ( i == (5 + offset) + 1 )
					{
						printf(" %-25s |", response_tokens[5+offset+1]);
					}
					else if ( i == (5 + offset) + 2 )
					{
						printf(" %-25s |", response_tokens[5+offset+2]);
					}
					else if ( i == (5 + offset) + 3 )
					{
						printf(" %-25s |", response_tokens[5+offset+3]);
					}
					else if ( i == (5 + offset) + 4 )
					{
						printf(" %-25s |\n", response_tokens[5+offset+4]);
					}

					iterations++;
					if ( (iterations % 5) == 0 )
					{
						offset += 5;
						printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
					}
				}
			}
			return 1;
		}
		printf("%s\n", "[ERROR]: An internal error has occured while executing this command. Contact with admin.");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*COMMAND_INVALID_PARAMS") == 0 )
	{	
		if ( 
			 (strcmp(response_tokens[1], "CHGPWD") == 0) ||
			 (strcmp(response_tokens[1], "KICK") == 0) || 
			 (strcmp(response_tokens[1], "MUTE") == 0) || 
			 (strcmp(response_tokens[1], "UNMUTE") == 0) || 
			 (strcmp(response_tokens[1], "PM") == 0) || 
			 (strcmp(response_tokens[1], "DEOP") == 0) ||
			 (strcmp(response_tokens[1], "OP") == 0) ||
			 (strcmp(response_tokens[1], "RECORD") == 0) ||
			 (strcmp(response_tokens[1], "STATUS") == 0) ||
			 (strcmp(response_tokens[1], "TABLE") == 0) ||
			 (strcmp(response_tokens[1], "BAN") == 0) ||
			 (strcmp(response_tokens[1], "UNBAN") == 0)
		   ) 
		{
			printf("%s", "Incorrect command usage. ");

			if ( strcmp(response_tokens[2], "TOO_MUCH_ARGS" ) == 0 )
			{
				printf("%s\n", "Number of arguments too much or few than command needs.");
				return 1;
			}
			if ( strcmp(response_tokens[2], "SELF_USE") == 0 )
			{
				printf("%s\n", "You can not apply this command to yourself!");
				return 1;
			}
			if ( strcmp(response_tokens[2], "INCORRECT_USERNAME") == 0 )
			{
				printf("%s\n", "Check property of username");
				return 1;
			}
			if ( strcmp(response_tokens[2], "USER_NOT_FOUND") == 0 )
			{
				printf("%s\n", "User not found in database file. Is it registered?");
				return 1;
			}
			if ( strcmp(response_tokens[2], "USER_OFFLINE") == 0 )
			{
				printf("%s\n", "Unable to execute a command. User is offline.");
				return 1;
			}
			if ( strcmp(response_tokens[2], "INCORRECT_TIME_VALUE") == 0 )
			{
				printf("%s\n", "Time argument is not a number!");
				return 1;
			}
			if ( strcmp(response_tokens[2], "INCORRECT_TIME_RANGE") == 0 )
			{
				printf("%s\n", "Time value in invalid range.");
				return 1;
			}
			if ( strcmp(response_tokens[2], "INCORRECT_STRING_VALUE") == 0)
			{
				printf("%s\n", "Your parameter has an incorrect value!");
				return 1;
			}
		}
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*COMMAND_PARAMS_NO_NEED") == 0 )
	{
		printf("%s\n", "This command should be executed without params");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*COMMAND_NO_PERMS") == 0 )
	{
		printf("%s\n", "You don't have permission to execute this command");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*UNKNOWN_COMMAND") == 0 )
	{
		printf("%s\n", "Unknown command. Type /help to see commands list");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*NO_PERM_TO_CREATE_FILE") == 0 )
	{
		printf("%s\n", "Unable to create file\n"
				       "Do you have permission to create files in this folder?");
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*SUCCESSFULLY_AUTHORIZED") == 0 )
	{
		fflush(stdout);
		clearScreen();
		printf("%s\n%s%s%s\n%s\n", "Welcome to GLOBAL chat room!",
				                   "You authorized here as \"", response_tokens[1],
								   "\"\nTo start chatting, just type text with ending ENTER",
								   "Type /help to show valid commands for your group"
			  );

		*authorized = 1;
		return 1;
	}

	return 0;
}
#ifdef _WIN32
int inet_ws_aton(const char* cp, struct in_addr* inp)
{
	if (cp == 0 || inp == 0)
		return 0;
	unsigned long addr = inet_addr(cp);
	if (addr == INADDR_NONE || addr == INADDR_ANY)
		return 0;

	inp->s_addr = addr;
	return 1;
}
#else
unsigned long long get_tick_unix(void)
{
	struct timespec cur_time;
	clock_gettime(CLOCK_REALTIME, &cur_time);

	return (cur_time.tv_sec*1000 + cur_time.tv_nsec/1000000);
}
#endif

#endif
