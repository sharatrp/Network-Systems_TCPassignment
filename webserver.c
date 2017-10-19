#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <stdlib.h>
#include <errno.h>

//#define Hardcode_on

#ifdef Hordcode_on
#define Port_addr 8081
#endif

int childsocket_no = 0;

int main(int argc , char *argv[])
{
	#ifndef Hardcode_on
	int Port_addr = atoi(argv[1]);
	#endif
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2048], command[10], URL[1024], child_no[256];
	char delim[] = " ";
	pid_t PID;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

	printf("Socket created at port: %d\n", Port_addr);

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(Port_addr);

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 15);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	while(1)
	{
		//accept connection from an incoming client. Wait till received
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}

		printf("\nConnection accepted\n");

		PID = fork();
		childsocket_no++;
		if(PID == 0)
		{
			close(socket_desc);

			if((read_size = read(client_sock , client_message , sizeof(client_message))) > 0 )
			{
				//printf("Client Message : %s\n", client_message);
			}
			printf("Child Socket: %d.\n", childsocket_no);
			bzero(command, sizeof(command));
			bzero(URL,sizeof(URL));

			strcpy(command, strtok(client_message, delim));
			strcpy(URL, strtok(NULL, delim));

			//printf("Command is : '%s'\n", command);
			printf("Child Socket: %d.URL is: '%s'\n", childsocket_no, URL);

			//printf("Creating a childing socket\n");
			if(!strcmp(URL, "/"))
			{		
				char writeBuffer[1024];	
				printf("Opening indeex file\n");
				system("pwd");
				FILE *fptr = fopen("index.html", "rb");
				if (fptr)
				{
					fseek(fptr, 0, SEEK_END);
					int filesize = ftell(fptr);
					fseek(fptr, 0, SEEK_SET);

					printf("file Size: %d\n", filesize);

					int sentSize = 0;

					char statusLine[256];
					sprintf(statusLine, "HTTP/1.1 200 OK\n\n");
					printf("statusLine : '%s'", statusLine);
					write(client_sock, statusLine, strlen(statusLine));
					//send(client_sock, statusLine, strlen(statusLine), 0);
					//memset(writeBuffer, 0, sizeof(writeBuffer));
					#if 1
					while (sentSize < filesize)
					{
						memset(writeBuffer, '\0', sizeof(writeBuffer));
						//Receive a message from client
						int read = fread(writeBuffer, sizeof(char), sizeof(writeBuffer), fptr);
						write(client_sock, writeBuffer, read);
						sentSize += read;
						printf("Read %d out of %d bytes\n", sentSize, filesize);
					}
					#else
					fread(writeBuffer, sizeof(char), filesize, fptr);
					write(client_sock, writeBuffer, strlen(writeBuffer));
					#endif
					printf("Sent Size: %d\n", sentSize);
					fclose(fptr);

				}
				else
				{
					int errsv = errno;
					printf("File Pointer: '%p'\n",fptr);
					printf("fopen failed. Error number: %d\n", errsv);				}
			}
			else 
			{
				char writeBuffer[1024];	
				memcpy(URL, URL+1, strlen(URL));
				printf("Child Socket: %d.Opening '%s' file\n", childsocket_no, URL);
				FILE *fptr = fopen(URL, "rb");
				if (fptr)
				{

					
					printf("Child Socket: %d.File Pointer: '%p'\n", childsocket_no,fptr);
					fseek(fptr, 0, SEEK_END);
					int filesize = ftell(fptr);
					fseek(fptr, 0, SEEK_SET);

					printf("Child Socket: %d.file Size: %d\n", childsocket_no, filesize);

					int sentSize = 0;

					char statusLine[256];
					//bzero(statusLine, sizeof(statusLine));
					memcpy(statusLine, "HTTP/1.1 200 OK\n\n", strlen("HTTP/1.1 200 OK\n\n"));
					write(client_sock, statusLine, strlen(statusLine));
					printf("Child Socket: %d.statusLine : '%s'", childsocket_no, statusLine);

					while (sentSize < filesize)
					{
						memset(writeBuffer, '\0', sizeof(writeBuffer));
						//Receive a message from client
						fread(writeBuffer, sizeof(char), sizeof(writeBuffer), fptr);
						write(client_sock, writeBuffer, strlen(writeBuffer));
						sentSize += strlen(writeBuffer);
					}
					printf("Child Socket: %d. EOF. Sent Size: %d\n", childsocket_no, sentSize);
					fclose(fptr);				

				}
				else
				{
					int errsv = errno;
					printf("File Pointer: '%p'\n",fptr);
					printf("fopen failed. Error number: %d\n", errsv);
				}
			}
			printf("Closing the child socket %d\n", childsocket_no);
			close(client_sock);  //Closing Child socket
			exit(0);
		}
		else if (PID > 0)
		{
			close(client_sock);
		}
		else
		{
			printf("fork failed\n");
		}
	}
}