#ifndef COMMON_API_C
#define COMMON_API_C

#include "commonAPI.h"
#include "dateTime.h"

const char* server_codes_list[SERVER_CODES_COUNT] = {
									"*CANNOT_CONNECT_DATABASE",
									"*CHGPWD_COMMAND_SUCCESS",
									"*CHGPWD_COMMAND_INCORRECT_PASS",
									"*CLIENT_HAS_ACCOUNT",
									"*CMD_ARG_OVERLIMIT_LENGTH",
									"*COMMAND_INVALID_PARAMS",
									"*COMMAND_NO_PERMS",
									"*COMMAND_PARAMS_NO_NEED",
									"*DEOP_COMMAND_SUCCESS",
									"*DEOP_COMMAND_USER_ALREADY_USER",
									"*HELP_COMMAND_SUCCESS",
									"*KICK_COMMAND_SUCCESS",
									"*LOGIN_WAIT_LOGIN",
									"*LOGIN_ALREADY_AUTHORIZED",
									"*LOGIN_ALREADY_USED",
									"*LOGIN_INCORRECT",
									"*LOGIN_NOT_EXIST",
									"*LOGIN_WAIT_PASS",
									"*MUTE_COMMAND_USER_ALREADY_MUTED",
									"*MUTE_COMMAND_SUCCESS",
									"*MUTE_COMMAND_YOU_MUTED",
									"*NEW_PASS_INCORRECT",
									"*NO_PERM_TO_CREATE_FILE",
									"*OP_COMMAND_SUCCESS",
									"*OP_COMMAND_USER_ALREADY_ADMIN",
									"*PASS_NOT_MATCH",
									"*RECORD_COMMAND_SUCCESS",
									"*SIGNUP_WAIT_LOGIN",
									"*SIGNUP_WAIT_PASS",
									"*STATUS_COMMAND_INCORRECT_STATUS",
									"*STATUS_COMMAND_ALREADY_SET",
									"*STATUS_COMMAND_SUCCESS",
									"*SUCCESSFULLY_AUTHORIZED",
									"*TABLE_COMMAND_SUCCESS",
									"*UNKNOWN_COMMAND",
									"*UNMUTE_COMMAND_USER_NOT_MUTED",
									"*UNMUTE_COMMAND_SUCCESS",
									"*UNMUTE_COMMAND_YOU_UNMUTED",
									"*USER_LEFT_CHAT",
									"*USER_AUTHORIZED",
									"*WHOIH_COMMAND_SUCCESS"
								};

void clearScreen(void)
{
	int i;
	for (i = 1; i <= 10; i++)
		printf("%s", "\n\n\n\n\n\n\n\n\n\n");
}
void clear_stdin(void)
{
	int c;
	do
		c = getchar();
	while (c != EOF && c != '\n');
}
void printUserRecord(char** args, unsigned int args_size)
{
	if (args_size != 7)
	{
		fprintf(stderr, "[%s] [ERROR]: Unexpected behaviour has occured while printing record\n", getCurTimeAsString());
		return;
	}

	printf("%s"
		   "%40s%-34s|\n"
		   "%s\n"
		   "| %17s | %-52s |\n"
		   "%s\n"
		   "| %17s | %-52s |\n"
		   "%s\n"
		   "| %17s | %-52s |\n"
		   "%s\n"
		   "| %17s | %-52s |\n"
		   "%s\n"
		   "| %17s | %-52s |\n"
		   "%s\n"
		   "| %-72s |\n"
		   "\\%74s/\n",
						"/--------------------------------------------------------------------------\\\n|",
														args[0], " record",
						"|--------------------------------------------------------------------------|",
														"Status", args[1],
						"|--------------------------------------------------------------------------|",
														"Rank", args[2],
						"|--------------------------------------------------------------------------|",
														"Real Name", args[3],
						"|--------------------------------------------------------------------------|",
														"Age", args[4],
						"|--------------------------------------------------------------------------|",
														"Registration Date", args[5],
						"|--------------------------------------------------------------------------|",
																args[6],
						 "--------------------------------------------------------------------------"
		  );
}
void printDebugRecord(char** args, unsigned int args_size)
{
	if (args_size != 17)
	{
		fprintf(stderr, "[%s] [ERROR]: Unexpected behaviour has occured while printing record\n", getCurTimeAsString());
		return;
	}
	
	int bufID = atoi(args[1]);
	int bufAuth = atoi(args[2]);
	int bufUsed = atoi(args[3]);
	int bufRank = atoi(args[9]);
	int bufSock = atoi(args[10]);
	int bufState = atoi(args[11]);
	int bufStatus = atoi(args[12]);
	int bufMuted = atoi(args[13]);
	int bufMuteTime = atoi(args[14]);
	int bufMuteTimeLeft = atoi(args[15]);
	int bufStartMuteTime = atoi(args[16]);

	printf("%s"
			"%40s%-34s|\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %1d %-50s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-52s |\n"
			"%s\n"
			"| %17s | %-52s |\n"
			"%s\n"
			"| %17s | %-52s |\n"
			"%s\n"
			"| %17s | %-52s |\n"
			"%s\n"
			"| %17s | %-52s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-4d %-47s |\n"
			"%s\n"
			"| %17s | %-15d %-36s |\n"
			"\\%74s/\n",
						"/--------------------------------------------------------------------------\\\n|",
														args[0], " record",
						"|--------------------------------------------------------------------------|",
														"ID", bufID, " ",
						"|--------------------------------------------------------------------------|",
														"Authorized", bufAuth, " ",
						"|--------------------------------------------------------------------------|",
														"Buffer used", bufUsed, " ",
						"|--------------------------------------------------------------------------|",
														"Last Date In", args[4],
						"|--------------------------------------------------------------------------|",
														"Last IP", args[5],
						"|--------------------------------------------------------------------------|",
														"Reg. Date", args[6],
						"|--------------------------------------------------------------------------|",
														"Login", args[7],
						"|--------------------------------------------------------------------------|",
														"Pass", args[8],
						"|--------------------------------------------------------------------------|",
														"Rank", bufRank, " ",
						"|--------------------------------------------------------------------------|",
														"Socket number", bufSock, " ",
						"|--------------------------------------------------------------------------|",
														"State", bufState, " ",
						"|--------------------------------------------------------------------------|",
														"Status", bufStatus, " ",
						"|--------------------------------------------------------------------------|",
						                                "Is muted", bufMuted, " ",
						"|--------------------------------------------------------------------------|",
						                                "Total mute time", bufMuteTime, " ",
						"|--------------------------------------------------------------------------|",
						                                "Mute time left", bufMuteTimeLeft," ",
						"|--------------------------------------------------------------------------|",
						                                "Start mute time", bufStartMuteTime, " ",
						 "--------------------------------------------------------------------------"
		  );
}
	
#endif
