#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>


#define N 10
#define SIZE_BUF 512

int main(int argc, char* arvg[])
{

	char Buff[SIZE_BUF];
	int sock = 0;
	char check[128];
	int tmp;
	int k;
	int authorization_flag = 0;
	struct sockaddr_in server;

	FILE* file;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1)
		printf("Could not create socket");
	puts("Socket create");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(8880);

	if (connect(sock, (struct sockaddr*) & server, sizeof(server)) < 0)
	{
		perror("Connect failed");
		return 1;
	}
	puts("Connect done");



	recv(sock, &Buff, SIZE_BUF, 0);     //CONNECTIONS
	printf("%s\n", Buff);



	fgets(Buff);
	if (strstr(Buff, "LIST") != NULL) // show directory
	{
		send(sock, &Buff, SIZE_BUF, 0);
		while (recv(sock, &Buff, SIZE_BUF, 0) != 0)
		{

			if (strcmp(Buff, "END") == 0)
			{
				printf("226 Transfer complete.\n"); //finish code?
				break;
			}
			printf("%s", Buff);
		}
	}

	if (strstr(Buff, "RETR") != NULL) // show directory
	{
		send(sock, &Buff, SIZE_BUF, 0);
		recv(sock, &Buff, SIZE_BUF, 0); //file name or error
		printf("%s", Buff);
		if (strstr(Buff, "END") != NULL)
		{
			printf("505. ERROR.\n");
		}
		else
		{
			//recv(sock, &Buff, 1024, 0);   
			file = fopen(Buff, "w");
			while (recv(sock, &Buff, SIZE_BUF, 0) != 0)
			{
				if (strcmp(Buff, "END") == 0)
				{
					printf("226 Transfer complete.\n");
					break;
				}
				fputs(Buff, file);
			}
			fclose(file);
		}
	}

	if (strstr(Buff, "STOR") != NULL) // show directory
	{
		send(sock, &Buff, SIZE_BUF, 0);

		tmp = strlen("STOR") + 1;
		for (k = 0; k < strlen(Buff) + 1; k++)
			check[k] = Buff[tmp++];

		file = fopen(check, "r");
		for (k = 0; (tmp = getc(file)) != EOF; k++)
		{
			Buff[k] = (char)tmp;
			if (k == SIZE_BUF - 2)
			{
				Buff[k + 1] = '\0';
				send(sock, &Buff, SIZE_BUF, 0);
				k = 0;
			}
		}
		Buff[k] = '\0';
		send(sock, &Buff, SIZE_BUF, 0);

		snprintf(Buff, SIZE_BUF, "END");
		send(sock, &Buff, SIZE_BUF, 0);
		fclose(file);
	}

	if (strstr(Buff, "DELE") != NULL) // show directory
	{
		send(sock, &Buff, SIZE_BUF, 0);
	}

	if (strstr(Buff, "QUIT") != NULL) // show directory
	{
		send(sock, &Buff, SIZE_BUF, 0);
		authorization_flag = 0;

		recv(sock, &Buff, SIZE_BUF, 0); //ANSWER QUIT
		printf("%s\n", Buff);
	}
}

	close(sock);
	return 0;
}