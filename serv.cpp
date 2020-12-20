#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>

#define N 10
#define SIZE_BUF 512

int main(int argc, char* argv[])
{
	//login & password
	char login[N][N], password[N][N];
	int log_pass_counter = 0;
	char check[128];
	int login_FLAG = 0;
	int password_FLAG = 0;
	char trash;
	int login_id;
	FILE* file_login, * file_password, * file;
	char directory_name[128] = "./";
	char file_name[128];
	int authorization_flag = 0;
	int k;
	DIR* dir;
	struct dirent* entry;

	char Buff[SIZE_BUF];
	int tmp;
	int socket_desc, c, client_sock, read_size;
	struct sockaddr_in server, client;

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		return 1;
	}
	puts("Socket create");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8880);
	if (bind(socket_desc, (struct sockaddr*) & server, sizeof(server)) < 0)
	{
		perror("Bind failed");
		return 1;
	}
	puts("Bind done");
	listen(socket_desc, 100);
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	client_sock = accept(socket_desc, (struct sockaddr*) & client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("Accept failed");
		return 1;
	}
	puts("Connection accept");

	///  command processing

	snprintf(Buff, SIZE_BUF, "Welcome to the FTP Server");




		strcpy(directory_name, "./");
		snprintf(Buff, SIZE_BUF, "Guest login");
		send(client_sock, &Buff, SIZE_BUF, 0);
		while (authorization_flag == 1)
		{
			recv(client_sock, &Buff, SIZE_BUF, 0);

			if (strstr(Buff, "LIST") != NULL) // show directory
			{

				dir = opendir(directory_name);
				if (!dir)
				{
					perror("diropen");
				}
				while ((entry = readdir(dir)) != NULL)
				{
					printf("%10s [%4d] %8d\n", entry->d_name, entry->d_type, entry->d_reclen);
					if (entry->d_type == 8) // 8 - type of file(txt, c, cpp, ...)
					{
						snprintf(Buff, SIZE_BUF, "%s [%d] %d\n", entry->d_name, entry->d_type, entry->d_reclen);
						send(client_sock, &Buff, SIZE_BUF, 0);
					}
				}
				closedir(dir);
				//end sending
				snprintf(Buff, SIZE_BUF, "END");
				send(client_sock, &Buff, SIZE_BUF, 0);
			}

			if (strstr(Buff, "RETR") != NULL) // transfer file from server to client
			{
				tmp = strlen("RETR") + 1;
				for (k = 0; k < strlen(Buff) + 1; k++)
					check[k] = Buff[tmp++];
				dir = opendir(directory_name);
				while ((entry = readdir(dir)) != NULL)
				{
					if ((entry->d_type == 8) && (strcmp(check, entry->d_name) == 0)) // 8 - type of file(txt, c, cpp, ...)
					{

						//strcpy(file_name, directory_name);
						//strcat(file_name, "/");                       
						//strcat(file_name, check);

						strcpy(Buff, check);
						send(client_sock, &Buff, SIZE_BUF, 0); // send filename

						strcpy(file_name, directory_name);
						strcat(file_name, "/");
						strcat(file_name, check);

						file = fopen(file_name, "r");

						for (k = 0; (tmp = getc(file)) != EOF; k++)
						{
							Buff[k] = (char)tmp;
							if (k == SIZE_BUF - 2)
							{
								send(client_sock, &Buff, SIZE_BUF, 0);
								k = 0;
							}
						}
						Buff[k] = '\0';
						send(client_sock, &Buff, SIZE_BUF, 0);

						snprintf(Buff, SIZE_BUF, "END");
						send(client_sock, &Buff, SIZE_BUF, 0);
						fclose(file);
					}
				}
				closedir(dir);
			}

			if (strstr(Buff, "STOR") != NULL) // transfer file from client to server
			{
				tmp = strlen("STOR") + 1;
				for (k = 0; k < strlen(Buff) + 1; k++)
					check[k] = Buff[tmp++];
				printf("%s", check);

				strcpy(file_name, directory_name);
				strcat(file_name, "/");
				strcat(file_name, check);

				file = fopen(file_name, "w");
				while (recv(client_sock, &Buff, SIZE_BUF, 0) != 0)
				{
					if (strcmp(Buff, "END") == 0)
					{
						printf("Recive is over.\n");
						break;
					}
					fputs(Buff, file);
					printf("%s\n", Buff);
				}
				fclose(file);
			}

			if (strstr(Buff, "DELE") != NULL) // delete file
			{
				tmp = strlen("DELE") + 1;
				for (k = 0; k < strlen(Buff) + 1; k++)
					check[k] = Buff[tmp++];
				printf("%s", check);
				strcpy(file_name, directory_name);
				strcat(file_name, "/");
				strcat(file_name, check);
				remove(file_name);
			}

			if (strstr(Buff, "QUIT") != NULL) // exit
			{
				snprintf(Buff, SIZE_BUF, "Quit succesful.");
				send(client_sock, &Buff, SIZE_BUF, 0);
				authorization_flag = 0;
			}
		}
	


	close(socket_desc);
	return 0;
}