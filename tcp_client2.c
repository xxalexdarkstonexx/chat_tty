#include "clientCore.h"
#include "commonAPI.h"
#include "dateTime.h"

#define MAX_RESPONSE_ARGS_NUM 100

extern const char* server_codes_list[SERVER_CODES_COUNT];

int main(int argc, char** argv)
{
	int authorized = 0;
	int messagesCounter = 0;
	int start_signal = 0;
	time_t startTime;
	time_t totalTime = AS_TOTAL_TIME_MS;

	clearScreen();
#ifdef _WIN32
	WSADATA d;
	if ( WSAStartup(MAKEWORD(2,2), &d) )
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif
	if (argc < 3)
	{
		fprintf(stderr, "usage: <program_name> <ip/hostname> <port>\n");
		return 1;
	}
	
	int ok;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
#ifdef _WIN32
	ok = inet_ws_aton(argv[1], &(addr.sin_addr));
#else
	ok = inet_aton(argv[1], &(addr.sin_addr));
#endif
	if (!ok)
	{
		fprintf(stderr, "Incorrect IP address\n");
		return 1;
	}
	unsigned int portNumber = atoi(argv[2]);
	if (portNumber < 1 || (portNumber >= 1 && portNumber <= 1024) )
	{
		fprintf(stderr, "Incorrect port number\n");
		return 1;
	}
	addr.sin_port = htons(portNumber);
	

	printf("%s\n", "Creating socket..");
	int peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (peer_sock == -1)
	{
		fprintf(stderr, "socket() failed. {%d}\n", GETSOCKETERRNO());
		return 1;
	}

	printf("%s\n", "Connecting..");
	if ( connect(peer_sock, (struct sockaddr*) &addr, sizeof(addr)) == -1 )
	{
		fprintf(stderr, "connect() failed. {%d}\n", GETSOCKETERRNO());
		return 1;
	}
	printf("%s\n", "Connected.");
	fflush(stdout);
	clearScreen();
	

	while (1)
    {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(peer_sock, &readfds);
#ifndef _WIN32
		FD_SET(0, &readfds);
#endif
		
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 300000;
		
		int res = select(peer_sock+1, &readfds, 0, 0, &timeout);

		if (res == -1)
		{
			fprintf(stderr, "select() failed. {%d}\n", GETSOCKETERRNO());
			return 1;
		}

		if (FD_ISSET(peer_sock, &readfds))
		{
			char* response_tokens[MAX_RESPONSE_ARGS_NUM];
			int i;
			for (i = 0; i < MAX_RESPONSE_ARGS_NUM; i++)
				response_tokens[i] = NULL;

			char read_buf[BUFSIZE];
			int bytes_received = recv(peer_sock, read_buf, BUFSIZE, 0);

			char* time_tokens[5];
			setTimeTokens(time_tokens);
			if ( bytes_received < 1 )
			{
				fprintf(stderr, "Connection closed by server.\n");
				break;
			}

			if ( bytes_received > 1 )
				read_buf[bytes_received-2] = '\0';

			char* istr = strtok(read_buf, "|");
			i = 0;
			while ( istr )
			{
				response_tokens[i] = istr;
				i++;
				istr = strtok(NULL, "|");
			}
			unsigned int response_tokens_size = i;
			
			/*
			printf("response_tokens_size = %d\n", response_tokens_size);
			for (i = 0; i < response_tokens_size; i++)
				printf("%s\n", response_tokens[i]);
			*/

			if ( checkServerResponse(response_tokens, response_tokens_size, peer_sock, &authorized) )
			{
				printf("Received %d bytes\n", bytes_received);
				continue;
			}

			printf("Received %d bytes\n", bytes_received);
			printf("<<< [%s] %s (%s) => %s\n", time_tokens[3], response_tokens[0], response_tokens[1], response_tokens[2]);
		}
#ifdef _WIN32
		if ( _kbhit() && authorized )
#else
		if ( FD_ISSET(0, &readfds) && authorized )
#endif
		{
			char send_buf[BUFSIZE];
			do
			{
				if ( !fgets(send_buf, BUFSIZE, stdin) ) return 0;
				deleteExtraSpaces(send_buf);
			}
			while ( (send_buf[0] == '\n') || (send_buf[0] == '\0') );
			
			int sent_bytes = restrictMessageLength(send_buf);
			
			/*spam-checking module*/
			time_t timestamps[2];
			time_t interval;
			time_t x;

			if ( !start_signal )
			{
				startTime = TIMER_START();
				start_signal = 1;
			}
			
			timestamps[(x%2)] = TIMER_START();
			x++;
			interval = timestamps[1] - timestamps[0];
			if (interval < 0) interval *= -1;
			
			sendall(peer_sock, send_buf, &sent_bytes);
			printf("Sent %d bytes.\n", sent_bytes);
			messagesCounter++;

			if ( messagesCounter >= AS_MSG_CNT )
			{
				messagesCounter = 0;
				start_signal = 0;
				totalTime = TIMER_START() - startTime;
			}
			
			if ( totalTime < AS_TOTAL_TIME_MS || interval < AS_MESSAGE_INTERVAL )
			{
				messagesCounter = 0;
				start_signal = 0;
				totalTime = AS_TOTAL_TIME_MS;
				unsigned int numberTries = 2;
				char* code;
				srand(time(0));
				
				printf("%s\n", "Before send next message, enter the following code(you have 2 tries)");
				do
				{
					char answer[100];
					code = getCode();
					printf("Your code: %s\n", code);

					fgets(answer, 6, stdin);
					answer[strlen(answer)-1] = '\0';

					if ( strcmp(answer, code) == 0 )
					{
						printf("%s\n", "You can continue to chat.");
						free(code);
						break;
					}
					free(code);
					numberTries--;
					if ( numberTries < 1 )
					{
						printf("%s\n", "You have failed spam-checking. Goodbye.");
						return 0;
					}
					printf("Wrong code! Try again. You have %d tries.\n", numberTries);
				}
				while ( numberTries > 0 );
			}
			/*printf("messagesCounter = %d\ntotalTime = %ld\ninterval = %ld\n", messagesCounter, totalTime, interval);*/
		}
    }

	printf("%s\n", "Closing socket..");
	CLOSESOCKET(peer_sock);
#ifdef _WIN32
	WSACleanup();
#endif
	printf("%s\n", "Finished");
	clear_stdin();
	fflush(stdout);
	return 0;
}
