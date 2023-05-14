#include "serverCore.h"

int main(int argc, char** argv)
{
	clearScreen();
#ifdef _WIN32
	WSADATA d;
	if ( WSAStartup(MAKEWORD(2,2), &d) )
	{
		fprintf(stderr, "Failed to initialize WinSock\n");
		return 1;
	}
#endif
	if (argc < 2)
	{
		fprintf(stderr, "usage: <program_name> <port>\n");
		return 1;
	}

	int portNumber = atoi(argv[1]);
	if (portNumber < 1 || (portNumber >= 1 && portNumber <= 1023) )
	{
		fprintf(stderr, "Incorrect port number\n");
		return 2;
	}
	
	if ( !server_init(portNumber) )
		return 3;
	
	int ret_value = server_running();
	
#ifdef _WIN32
	WSACleanup();
#endif
	printf("%s\n", "Finished.");

	return ret_value;
}
