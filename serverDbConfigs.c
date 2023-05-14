#ifndef SERVERDBCONFIGS_C_SENTRY
#define SERVERDBCONFIGS_C_SENTRY

#include "dateTime.h"
#include "serverCore.h"
#include "serverCommands.h"
#include "serverDbConfigs.h"
#include <time.h>

enum
{
	CUR_TIME_SIZE = 100,
	CUR_DATE_BUF_SIZE = 100
};

extern Server* serv;

static int parse_configuration_file(int* records_size)
{
	char config_strings[CONFIG_STRINGS_NUM][CONFIG_STRING_SIZE];
	FILE* cfgPtr;
	int f_closed = 0;
	char cur_time[CUR_TIME_SIZE];

	if ( !(cfgPtr = fopen(CONFIG_NAME, "r")) )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to open file \"%s\". Creating new one..\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_NAME);

		if ( !(cfgPtr = fopen(CONFIG_NAME, "w")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to create \"%s\" file. Do you have permission to this?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_NAME);
			return 3;
		}
		fprintf(stderr, "[%s] [INFO]: Setting all configs to default values..\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		fprintf(cfgPtr, "currentDbSize=10");
		fclose(cfgPtr);
		f_closed = 1;
	}

	if (f_closed)
		if ( !(cfgPtr = fopen(CONFIG_NAME, "r")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_NAME);
			return 3;
		}
	
	int i = 0;
	while ( !feof(cfgPtr) )
	{
		fgets(config_strings[i], CONFIG_STRING_SIZE, cfgPtr);
		i++;
		if (i >= CONFIG_STRINGS_NUM)
			break;
	}
	fclose(cfgPtr);

	char* parsed_cfg_setting[2];
	char* istr = strtok(config_strings[0], "=");

	i = 0;
	while (istr)
	{
		parsed_cfg_setting[i] = istr;
		i++;
		istr = strtok(NULL, "=");
		if (i > 1)
			break;
	}
	
	i = 0;
	while ( parsed_cfg_setting[0][i] )
	{
		if ( parsed_cfg_setting[0][i] == '\n' )
			parsed_cfg_setting[0][i] = '\0';
		i++;
	}

	if ( !strcmp(parsed_cfg_setting[0], "currentDbSize") )
	{
		if ( !(*records_size = atoi(parsed_cfg_setting[1])) )
			return 1;
		if (*records_size < 1)
			return 2;
	}
	else
		return 1;

	return 0;
}
/*char* getLogFilename(char* filename_buf, unsigned int buf_size)
{
	if (buf_size < 100)
		buf_size = 100;

	const char* suffix = "_log.txt";
	
	time_t cur_time_in_secs = time(0);
	struct tm* time = NULL;
	time = localtime(&cur_time_in_secs);

	strftime(filename_buf, buf_size, "%d_%m_%y_%H_%M_%S", time);

	int cur_pos = strlen(filename_buf);
	int i;
	for (i = 0; suffix[i]; i++, cur_pos++)
		filename_buf[cur_pos] = suffix[i];
	filename_buf[cur_pos] = '\0';
	
	return filename_buf;
}*/
void evaluate_size_db(int* records_size)
{
	char cur_time[CUR_TIME_SIZE];
	int cfg_error_code;
	if ( (cfg_error_code = parse_configuration_file(records_size)) )
	{
		switch (cfg_error_code)
		{
			case 1:
				printf("[%s] [ERROR]: Error has occured while attempting to parse \"%s\" file\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_NAME);
				printf("[%s] [ERROR]: Set default value to %d\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_SETTING_DEFAULT_SIZE);
				*records_size = CONFIG_SETTING_DEFAULT_SIZE;
				break;
			case 2:
				printf("[%s] [ERROR]: Incorrect value of \"currentDbSize\" parameter\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
				printf("[%s] [ERROR]: Set default value to %d\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), CONFIG_SETTING_DEFAULT_SIZE);
				*records_size = CONFIG_SETTING_DEFAULT_SIZE;
				break;
			case 3:
				printf("[%s] [ERROR]: You don't fave permission to create or open file in this directory!\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
				*records_size = CONFIG_SETTING_DEFAULT_SIZE;
		}
	}
}
void initUserInfoDbFile(int records_num, int extension)
{
	FILE* dbusers;
	const char* empty_field = "undefined";
	char cur_time[CUR_TIME_SIZE];
	
	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		if ( !(dbusers = fopen(DB_USERINFO_NAME, "wb")) )
		{
			fprintf(stderr, "[%s] [ERROR]: You don't have permission to create file in this directory.\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
			return;
		}
		
		int i;
		for (i = 1; i <= records_num; i++)
		{
			DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));

			buf->ID = -1;
			int j;
			for (j = 0; empty_field[j]; j++)
			{
				buf->username[j] = empty_field[j];
				buf->pass[j] = empty_field[j];
			}
			buf->username[j] = '\0';
			buf->pass[j] = '\0';

			buf->rank[0] = 'u';
			buf->rank[1] = '\0';
			
			buf->age = -1;
			buf->realname[0] = '\0';
			buf->quote[0] = '\0';

			fwrite(buf, sizeof(DBUsersInformation), 1, dbusers);
			free(buf);
		}
	}
	fclose(dbusers);

	if ( extension )
	{
		if ( !(dbusers = fopen(DB_USERINFO_NAME, "ab")) )
		{
			fprintf(stderr, "[%s] [ERROR]: You don't have permission to create file in this directory.\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
			return;
		}

		int i;
		for	(i = 1; i <= records_num; i++)
		{
			DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));

			buf->ID = -1;
			int j;
			for (j = 0; empty_field[j]; j++)
			{
				buf->username[j] = empty_field[j];
				buf->pass[j] = empty_field[j];
			}
			buf->username[j] = '\0';
			buf->pass[j] = '\0';

			buf->rank[0] = 'u';
			buf->rank[1] = '\0';
			
			buf->age = -1;
			buf->realname[0] = '\0';
			buf->quote[0] = '\0';

			fwrite(buf, sizeof(DBUsersInformation), 1, dbusers);
			free(buf);
		}
		fclose(dbusers);
	}
}
void initExtUserInfoDbFile(int records_num, int extension)
{
	FILE* dbxusers;
	const char* empty_field = "undefined";
	char cur_time[CUR_TIME_SIZE];	

	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
	{
		if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "wb")) )
		{
			fprintf(stderr, "[%s] [ERROR]: You don't have permission to create file in this directory.\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
			return;
		}
		
		int i;
		for (i = 1; i <= records_num; i++)
		{
			DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));

			xbuf->ID = -1;
			int j;
			for (j = 0; empty_field[j]; j++)
			{
				xbuf->lastIP[j] = empty_field[j];
				xbuf->lastDateIn[j] = empty_field[j];
				xbuf->registrationDate[j] = empty_field[j];
				xbuf->lastDateOut[j] = empty_field[j];
			}
			xbuf->lastIP[j] = '\0';
			xbuf->lastDateIn[j] = '\0';
			xbuf->registrationDate[j] = '\0';
			xbuf->lastDateOut[j] = '\0';
			
			fwrite(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
			free(xbuf);
		}
	}
	fclose(dbxusers);

	if ( extension )
	{
		if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "ab")) )
		{
			fprintf(stderr, "[%s] [ERROR]: You don't have permission to create file in this directory.\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
			return;
		}

		int i;
		for (i = 1; i <= records_num; i++)
		{
			DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));

			xbuf->ID = -1;
			int j;
			for (j = 0; empty_field[j]; j++)
			{
				xbuf->lastIP[j] = empty_field[j];
				xbuf->lastDateIn[j] = empty_field[j];
				xbuf->registrationDate[j] = empty_field[j];
				xbuf->lastDateOut[j] = empty_field[j];
			}
			xbuf->lastIP[j] = '\0';
			xbuf->lastDateIn[j] = '\0';
			xbuf->registrationDate[j] = '\0';
			xbuf->lastDateOut[j] = '\0';
			
			fwrite(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);
			free(xbuf);
		}
		fclose(dbxusers);
	}
}
void initOpsFile(int records_size)
{
	FILE* dbops;
	FILE* dbusers;
	char** ops_strings = NULL;
	int stringsCount = 0;
	unsigned int isOpsEmpty = 0;
	char cur_time[CUR_TIME_SIZE];

	ops_strings = parseOpsFile(&stringsCount);
	if ( !ops_strings )
	{
		fprintf(stderr, "[%s] [WARN]: Unable to open \"ops.txt\"\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		fprintf(stderr, "[%s] [WARN]: File does not exist or you don't have permission to open in this dir\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		fprintf(stderr,	"[%s] [INFO]: Creating new file..\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));

		if ( !(dbops = fopen("ops.txt", "w")) )
		{
			fprintf(stderr, "[%s] [ERROR]: Unable to create file \"ops.txt\". Do you have permission for this?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
			return;
		}
		isOpsEmpty = 1;
	}
	
	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fclose(dbops);
		fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		return;
	}

	printf("[%s] [INFO]: File \"ops.txt\" has been successfully opened\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
	printf("[%s] [INFO]: Checking property of \"ops.txt\" records..\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));

	
	char** mismatched_users = NULL;
	int usersCount = 0;
	int not_admin;
	int i;
	for (i = 0; i < records_size; i++)
    {
		not_admin = 0;
		DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(buf, sizeof(DBUsersInformation), 1, dbusers);

		if (buf->rank[0] == 'A')
		{
			int flag = 0;
			if (!isOpsEmpty)
			{
				int j;
				for (j = 0; j < stringsCount; j++)
					if (strcmp(buf->username, ops_strings[j]) == 0)
					{
						flag = 1;
						break;
					}

				if (!flag)
				{
					usersCount++;
					int str_len = strlen(buf->username);

					if (mismatched_users == NULL)
					{
						mismatched_users = malloc(sizeof(char*) * usersCount);
						mismatched_users[usersCount-1] = malloc(sizeof(char)*str_len+1);
						int k;
						for (k = 0; buf->username[k]; k++)
							mismatched_users[usersCount-1][k] = buf->username[k];
						mismatched_users[usersCount-1][k] = '\0';
					}
					else
					{
						mismatched_users = realloc(mismatched_users, sizeof(char*) * usersCount);
						mismatched_users[usersCount-1] = malloc(sizeof(char)*str_len+1);
						int k;
						for (k = 0; buf->username[k]; k++)
							mismatched_users[usersCount-1][k] = buf->username[k];
						mismatched_users[usersCount-1][k] = '\0';
					}
				}
				free(buf);

				if ( (i+1) < records_size )
					continue;
				fclose(dbusers);

				if ( !(dbops = fopen("ops.txt", "w")) )
				{
					int j;
					for (j = 0; j < records_size; j++)
						free(ops_strings[j]);
					free(ops_strings);
					ops_strings = NULL;
					
					if (usersCount > 0)
					{
						for (j = 0; j < usersCount; j++)
							free(mismatched_users[j]);
						free(mismatched_users);
						mismatched_users = NULL;
					}

					fprintf(stderr, "[%s] [ERROR]: Unable to create file \"ops.txt\". Do you have permission for this?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
					return;
				}
				
				for (j = 0; j < stringsCount; j++)
				{
					fprintf(dbops, "%s\n", ops_strings[j]);
					free(ops_strings[j]);
				}
				free(ops_strings);
				ops_strings = NULL;

				if (usersCount > 0)
				{
					for (j = 0; j < usersCount; j++)
					{
						fprintf(dbops, "%s\n", mismatched_users[j]);
						free(mismatched_users[j]);
					}
					free(mismatched_users);
					mismatched_users = NULL;
				}
				fclose(dbops);
				break;
			}
			else
			{
				int str_len = strlen(buf->username);
				char* ops_string = malloc(sizeof(char)*str_len+1);
				int j;
				for (j = 0; buf->username[j]; j++)
					ops_string[j] = buf->username[j];
				ops_string[j] = '\0';

				fprintf(dbops, "%s\n", ops_string);
				free(ops_string);
			}
		}
		else if ( buf->rank[0] != 'A')
			not_admin = 1;

		free(buf);
	}
	if (isOpsEmpty)
	{
		fclose(dbops);
		fclose(dbusers);
	}
	
	if (not_admin)
	{
		int j;
		for (j = 0; j < stringsCount; j++)
			free(ops_strings[j]);
		free(ops_strings);
		ops_strings = NULL;
	}
	

	printf("[%s] [INFO]: Done!\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
}
int getDBRecordIndexByName(const char* nickname)
{
	FILE* dbusers;
	int records_size = 0;
	evaluate_size_db(&records_size);
	char cur_time[CUR_TIME_SIZE];

	if ( records_size == 0 )
	{
		fprintf(stderr, "[%s] [ERROR]: An internal error occured in \"getDBRecordIndexByName\" while attempting to get \"record_size\" param. It's 0\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE));
		return -2;
	}
	
	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		return -3;
	}

	int i;
	for (i = 0; i < records_size; i++)
	{
		DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(buf, sizeof(DBUsersInformation), 1, dbusers);

		if ( strcmp(buf->username, nickname) == 0 )
		{
			free(buf);
			break;
		}
		free(buf);
	}
	fclose(dbusers);

	if ( i == records_size )
	{
		return -1;
	}

	return i;
}
static FILE* getOpsDbFile(void)
{
	FILE* dbops;
	if ( (dbops = fopen("ops.txt", "r")) )
		return dbops;

	return 0;
}
char** parseOpsFile(int* stringsCount)
{
	FILE* dbops;
	*stringsCount = 0;
	int old_size;
	char** ops_strings;
	int* strings_length = NULL;

	if ( (dbops = getOpsDbFile()) )
	{
		int c;
		while ( (c = fgetc(dbops)) != EOF )
			if (c == '\n')
				(*stringsCount)++;
		fclose(dbops);
	}
	else
		return NULL;

	if (*stringsCount == 0)
		return NULL;

	if ( (dbops = getOpsDbFile()) )
	{
		int c;
		old_size = *stringsCount;
		strings_length = malloc( old_size*sizeof(int) );
		int i;
		for (i = 0; i < *stringsCount; i++)
			strings_length[i] = 0;

		i = 0;
		while ( (c = fgetc(dbops)) != EOF )
		{
			if (c != '\n')
				if (c != ' ')
					strings_length[i] += 1;
			if (c == '\n')
				i++;
		}

		for (i = 0; i < old_size; i++)
			if (strings_length[i] == 0)
				(*stringsCount)--;
		fclose(dbops);
	}
	else
		return NULL;

	if ( (dbops = getOpsDbFile()) )
	{
		int c;
		ops_strings = malloc(sizeof(char*) * (*stringsCount));

		int i, j = 0;
		for (i = 0; i < old_size; i++)
			if (strings_length[i] > 0)
			{
				ops_strings[j] = malloc(sizeof(char)*strings_length[i]+1);
				j++;
			}

		j = 0;
		i = 0;
		int k = 0;
		while ( (c = fgetc(dbops)) != EOF )
		{
			if (strings_length[k] < 1)
			{
				if (c == '\n')
					k++;
				continue;
			}
			
			if (c != '\n')
			{
				if (c == ' ')
					continue;
				ops_strings[i][j] = c;
				j++;
			}
			else
			{
				ops_strings[i][j] = '\0';
				j = 0;
				i++;
				k++;
			}
		}
		free(strings_length);
		strings_length = NULL;
		fclose(dbops);
	}
	else
		return NULL;

	return ops_strings;
}
void printDBUsers(void)
{
	FILE* dbusers;
	int records_size = 0;
	char cur_time[CUR_TIME_SIZE];

	evaluate_size_db(&records_size);

	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		return;
	}

	int i;
	printf("%s\n", "------------------------------------------------------");
	printf("| %-4s | %-16s | %-20s | %-1s |\n", "ID", "Username", "Password", "R");
	printf("%s\n", "------------------------------------------------------");
	for ( i = 0; i < records_size; i++ )
	{
		DBUsersInformation* buf = malloc(sizeof(DBUsersInformation));
		fseek(dbusers, i*sizeof(DBUsersInformation), SEEK_SET);
		fread(buf, sizeof(DBUsersInformation), 1, dbusers);

		printf("| %04d | %-16s | %-20s | %-1s |\n", buf->ID, buf->username, buf->pass, buf->rank);
		printf("%s\n", "------------------------------------------------------");
		free(buf);
	}
	fclose(dbusers);
}
void printDBXUsers(void)
{
	FILE* dbxusers;
	int records_size = 0;
	evaluate_size_db(&records_size);
	char cur_time[CUR_TIME_SIZE];

	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_XUSERINFO_NAME);
		return;
	}

	int i;
	printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
	printf("| %-4s | %-25s | %-25s | %-25s | %-25s |\n", "ID", "Reg. Date", "Last In Date", "Last Out Date", "Last IP");
	printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
	for ( i = 0; i < records_size; i++)
	{
		DBXUsersInformation* xbuf = malloc(sizeof(DBXUsersInformation));
		fseek(dbxusers, i*sizeof(DBXUsersInformation), SEEK_SET);
		fread(xbuf, sizeof(DBXUsersInformation), 1, dbxusers);

		printf("| %04d | %-25s | %-25s | %-25s | %-25s |\n", xbuf->ID, xbuf->registrationDate, xbuf->lastDateIn, xbuf->lastDateOut, xbuf->lastIP);
		printf("%s\n", "------------------------------------------------------------------------------------------------------------------------");
		free(xbuf);
	}
	fclose(dbxusers);
}
void updateDBUsersRecords(ClientSession *sess)
{
	FILE* dbusers;
	char cur_time[CUR_TIME_SIZE];
	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		sess->state = fsm_error;
		return;
	}

	DBUsersInformation* rbuf = malloc(sizeof(DBUsersInformation));
	fseek(dbusers, sess->ID*sizeof(DBUsersInformation), SEEK_SET);
	fread(rbuf, sizeof(DBUsersInformation), 1, dbusers);
	fclose(dbusers);

	if ( !(dbusers = fopen(DB_USERINFO_NAME, "rb+")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(cur_time, CUR_TIME_SIZE), DB_USERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		sess->state = fsm_error;
		return;
	}
	DBUsersInformation* bufUsers = malloc(sizeof(DBUsersInformation));

	bufUsers->ID = sess->ID;
	if ( bufUsers->ID != -1 )
	{
		bufUsers->age = rbuf->age;

		int i;
		if ( rbuf->realname[0] )
		{
			for ( i = 0; rbuf->realname[i]; i++ )
				bufUsers->realname[i] = rbuf->realname[i];
			bufUsers->realname[i] = '\0';
		}
		else
			bufUsers->realname[0] = '\0';
		
		if ( rbuf->quote[0] )
		{
			for ( i = 0; rbuf->quote[i]; i++ )
				bufUsers->quote[i] = rbuf->quote[i];
			bufUsers->quote[i] = '\0';
		}
		else
			bufUsers->quote[0] = '\0';
		free(rbuf);

		for ( i = 0; sess->login[i]; i++ )
			bufUsers->username[i] = sess->login[i];
		bufUsers->username[i] = '\0';

		for ( i = 0; sess->pass[i]; i++ )
			bufUsers->pass[i] = sess->pass[i];
		bufUsers->pass[i] = '\0';

		bufUsers->rank[0] = getUserRank(sess);
		bufUsers->rank[1] = '\0';

		fseek(dbusers, sess->ID*sizeof(DBUsersInformation), SEEK_SET);
		fwrite(bufUsers, sizeof(DBUsersInformation), 1, dbusers);
	}
	free(bufUsers);
	fclose(dbusers);
}
void updateDBXUsersRecords(ClientSession *sess)
{
	FILE* dbxusers;
	char cur_date[CUR_TIME_SIZE];
	if ( !(dbxusers = fopen(DB_XUSERINFO_NAME, "rb+")) )
	{
		fprintf(stderr, "[%s] [ERROR]: Unable to open database file \"%s\". Is it exist?\n", getCurTimeAsString(cur_date, CUR_TIME_SIZE), DB_XUSERINFO_NAME);
		session_send_string(sess, "*CANNOT_CONNECT_DATABASE\n");
		sess->state = fsm_error;
		return;
	}
	DBXUsersInformation* bufXUsers = malloc(sizeof(DBXUsersInformation));
	bufXUsers->ID = sess->ID;
	if ( bufXUsers->ID != -1 )
	{
		int j;
		for ( j = 0; sess->lastIP[j]; j++ )
			bufXUsers->lastIP[j] = sess->lastIP[j];
		bufXUsers->lastIP[j] = '\0';

		for ( j = 0; sess->lastDateIn[j]; j++ )
			bufXUsers->lastDateIn[j] = sess->lastDateIn[j];
		bufXUsers->lastDateIn[j] = '\0';

		for ( j = 0; sess->registrationDate[j]; j++ )
			bufXUsers->registrationDate[j] = sess->registrationDate[j];
		bufXUsers->registrationDate[j] = '\0';

		bufXUsers->muted = sess->muted;
		bufXUsers->start_mute_time = sess->start_mute_time;
		bufXUsers->mute_time_left = sess->mute_time_left;
		bufXUsers->mute_time = sess->mute_time;

		if ( !sess->authorized )
		{
			char* undef = "undefined";
			for ( j = 0; undef[j]; j++ )
				bufXUsers->lastDateOut[j] = undef[j];
			bufXUsers->lastDateOut[j] = '\0';
		}
		else
		{
			char lastOut[100];
			getCurDateAsString(lastOut, CUR_DATE_BUF_SIZE);
			int i;
			for ( i = 0; lastOut[i]; i++ )
				bufXUsers->lastDateOut[i] = lastOut[i];
			bufXUsers->lastDateOut[i] = '\0';
		}
		
		fseek(dbxusers, bufXUsers->ID*sizeof(DBXUsersInformation), SEEK_SET);
		fwrite(bufXUsers, sizeof(DBXUsersInformation), 1, dbxusers);
	}
	free(bufXUsers);
	fclose(dbxusers);
}

#endif
