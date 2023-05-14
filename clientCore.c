#ifndef CLIENTCORE_C_SENTRY
#define CLIENTCORE_C_SENTRY

#include "commonAPI.h"
#include "clientCore.h"

enum
{
	MAX_STRING_LENGTH = 52,
	HAS_ACCOUNT_VALUE_LENGTH = 3,
};

extern int peer_sock;

static void show_logo(void)
{
	printf( "%s",  
					"   $$$$$$$$\\  $$$$$$\\  $$$$$$$\\        $$$$$$\\  $$\\   $$\\  $$$$$$\\ $$$$$$$$\\\n"
					"   \\__$$  __|$$  __$$\\ $$  __$$\\      $$  __$$\\ $$ |  $$ |$$  __$$\\\\__$$  __|\n"
					"      $$ |   $$ /  \\__|$$ |  $$ |     $$ /  \\__|$$ |  $$ |$$ /  $$ |  $$ |   \n"
					"      $$ |   $$ |      $$$$$$$  |     $$ |      $$$$$$$$ |$$$$$$$$ |  $$ |    \n"
					"      $$ |   $$ |      $$  ____/      $$ |      $$  __$$ |$$  __$$ |  $$ |    \n"
					"      $$ |   $$ |  $$\\ $$ |           $$ |  $$\\ $$ |  $$ |$$ |  $$ |  $$ |    \n"
					"      $$ |   \\$$$$$$  |$$ |           \\$$$$$$  |$$ |  $$ |$$ |  $$ |  $$ |    \n"
					"      \\__|    \\______/ \\__|            \\______/ \\__|  \\__|\\__|  \\__|  \\__|\n"
					"\n\n"
		  );
}
static void printHorizontalLine(unsigned int offset, unsigned int line_length, char char_line)
{
	int i;
	for (i = 1; i <= offset; i++)
		putchar(' ');
	for (i = 1; i <= line_length; i++)
		putchar(char_line);
    for (i = 1; i <= offset; i++)
		putchar(' ');
}
static void printGreetingTextFrame(const char** text_strings, int text_strings_size)
{
	printHorizontalLine(0, 14, ' ');
	printf("%s\n", "Welcome to authorization page of my simple TCP chat.");
	printHorizontalLine(14, MAX_STRING_LENGTH, '#');
	putchar('\n');

	int i = 0;
	for (; i < text_strings_size; i++)
	{
		int len = strlen(text_strings[i]);
		
		if ( len > MAX_STRING_LENGTH-4)
			len = MAX_STRING_LENGTH-4;

		printHorizontalLine(0, 14, ' ');
		printHorizontalLine(0, 2, '#');
		
		int isOdd = 0;
		int offset_len; 
		( ((offset_len = MAX_STRING_LENGTH-4-len) % 2) != 0 ) ? isOdd = 1 : isOdd; 
		offset_len /= 2;

		printHorizontalLine(0, offset_len, ' ');
		int j;
		for ( j = 0; j < len; j++ )
			putchar(text_strings[i][j]);
		( isOdd == 1 ) ? printHorizontalLine(0, offset_len+1, ' ') : printHorizontalLine(0, offset_len, ' ') ; 

		printHorizontalLine(0, 2, '#');
		putchar('\n');
	}

	printHorizontalLine(14, MAX_STRING_LENGTH, '#');
	putchar('\n');
	printHorizontalLine(0, 14, ' ');
	printf("%s", "Your answer: ");
}
static void sendAnswer(const char** box_messages, int box_messages_size, int max_read_chars)
{	
	char message[100];	/*it might be at least max_read_chars+2 bytes*/
	unsigned int input_chars_num = 0;
	int size = 0;


	clearScreen();
	show_logo();
	printGreetingTextFrame(box_messages, box_messages_size);

	do
	{
		size = get_string(message, max_read_chars, &input_chars_num);
	}
	while ( message[0] == '\n' );


	if ( input_chars_num > max_read_chars )
		clear_stdin();

	sendall(peer_sock, message, &size);
	printf("Sent %d bytes\n", size);
}

static int viewRecordSuccessResult(char** response_tokens, int fields_num, int debug_mode)
{
	char** args = malloc(sizeof(char*) * fields_num);
	if ( !args )
		return 0;

	int i, j, k;
	for ( i = 0, j = 3; i < fields_num; i++, j++ )
	{
		args[i] = malloc(sizeof(char) * strlen(response_tokens[j]) + 1 );
		if ( !(args[i]) )
		{
			int m;
			for (m = 0; m < i; m++)
				if ( args[m] )
					free(args[m]);
			return 0;
		}

		for ( k = 0; response_tokens[j][k]; k++ )
			args[i][k] = response_tokens[j][k];
		args[i][k] = '\0';
	}
	printRecord(args, fields_num, debug_mode);
	
	for ( i = 0; i < fields_num; i++ )
		if ( args[i] )
			free(args[i]);
	if ( args )
		free(args);

	return 1;
}

int checkServerResponse(char **response_tokens, unsigned int response_tokens_size, int* authorized)
{
	if ( strcmp(response_tokens[0], "*CLIENT_HAS_ACCOUNT") == 0 )
	{
		int max_read_chars = HAS_ACCOUNT_VALUE_LENGTH;
		const char* box_messages[] = { "Have you already have an account?", "Enter \"y\"or\"n\"", NULL };

		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_WAIT_LOGIN") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = { "Enter nickname for your account", NULL };
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_ALREADY_AUTHORIZED") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = { "This account is already authorized", "Try to use another login", NULL };
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_ALREADY_USED") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = {"This login is already exist in database", "Try another one",  NULL };
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_INCORRECT") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = {"Incorrect login!", "Please, check it and try again", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_NOT_EXIST") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = {"This login doesn't exist", "Check your input string", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*SIGNUP_WAIT_LOGIN") == 0 )
	{
		int max_read_chars = MAX_LOGIN_LENGTH;
		const char* box_messages[] = {"Enter nickname to create", "new account", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*LOGIN_WAIT_PASS") == 0 )
	{
		int max_read_chars = MAX_PASS_LENGTH;
		const char* box_messages[] = {"Enter password for your account", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*NEW_PASS_INCORRECT") == 0 )
	{
		int max_read_chars = MAX_PASS_LENGTH;
		const char* box_messages[] = {"Password incorrect!", "Check it and try again", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*PASS_NOT_MATCH") == 0 )
	{
		int max_read_chars = MAX_PASS_LENGTH;
		const char* box_messages[] = {"Password doesn't match with this account", "Try again", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*SIGNUP_WAIT_PASS") == 0 )
	{
		int max_read_chars = MAX_PASS_LENGTH;
		const char* box_messages[] = {"Enter pass for your new account", NULL};
		
		int box_messages_size = 0;
		while ( box_messages[box_messages_size] )
			box_messages_size++;

		sendAnswer(box_messages, box_messages_size, max_read_chars);
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*USER_AUTHORIZED") == 0 )
	{
		putchar('\n');
		int i;
		for ( i = 1; i <= 28; i++ )
			putchar(' ');
		printf("\"%s\" joined to GLOBAL chat\n", response_tokens[1]);

		return 1;
	}
	else if ( strcmp(response_tokens[0], "*USER_LEFT_CHAT") == 0 )
	{
		putchar('\n');
		int i;
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
			if ( !viewRecordSuccessResult(response_tokens, DEBUG_RECORD_FIELDS_NUM, 1) )
				printf("%s\n", "[ERROR]: An error has occured while allocating memory to string buffers!");
		}
		else if ( strcmp(response_tokens[1], "record") == 0 )
		{
			if ( !viewRecordSuccessResult(response_tokens, USER_RECORD_FIELDS_NUM, 0) )
				printf("%s\n", "[ERROR]: An error has occured while allocating memory to string buffers!");
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
		}
		else if ( strcmp(response_tokens[1], "VICTIM") == 0 )
		{
			clearScreen();
			printf("%s\n", "You have been kicked from the chat.");
		}
		return 1;
	}
	else if ( strcmp(response_tokens[0], "*TABLE_COMMAND_SUCCESS") == 0 )
	{
		/*
		int z;
		printf("%s\n", "DEBUG----------------------------------------DEBUG");
		for (z = 0; z < response_tokens_size; z++)
		{
			printf("\t\t%s\n", response_tokens[z]);
		}
		printf("%s\n", "DEBUG----------------------------------------DEBUG");
		*/


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
			int offset = 0;
			int iterations = 0;

			if ( strcmp(response_tokens[4], "USERINFO") == 0 )
			{
				printf(
					   "\n%s\n"
					   "%s\n"
					   "| %-4s | %-16s | %-20s | %-1s |\n"
					   "%s\n",
					   "File \"usersdata.dat\":",
					   "------------------------------------------------------",
					   "ID", "Username", "Password", "R",
					   "------------------------------------------------------"
					  );
				
				int i;
			    for ( i = 5; i < (5*non_empty_records_size+4); i++ )
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
				printf(
						"\n%s\n" 
					    "%s\n"
						"| %-4s | %-25s | %-25s | %-25s | %-25s |\n"
						"%s\n",
						"File \"users_sessions_info.dat\":",
						"------------------------------------------------------------------------------------------------------------------------",
						"ID", "Reg. Date", "Last In Date", "Last Out Date", "Last IP",
						"------------------------------------------------------------------------------------------------------------------------"
					  );

				int i;
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
			 (strcmp(response_tokens[1], "CHGPWD") == 0)	||
			 (strcmp(response_tokens[1], "KICK")   == 0)	|| 
			 (strcmp(response_tokens[1], "MUTE")   == 0)	|| 
			 (strcmp(response_tokens[1], "UNMUTE") == 0)	|| 
			 (strcmp(response_tokens[1], "PM")     == 0)	|| 
			 (strcmp(response_tokens[1], "DEOP")   == 0)	||
			 (strcmp(response_tokens[1], "OP")     == 0)	||
			 (strcmp(response_tokens[1], "RECORD") == 0)	||
			 (strcmp(response_tokens[1], "STATUS") == 0)	||
			 (strcmp(response_tokens[1], "TABLE")  == 0)	||
			 (strcmp(response_tokens[1], "BAN")    == 0)	||
			 (strcmp(response_tokens[1], "UNBAN")  == 0)
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
#endif
