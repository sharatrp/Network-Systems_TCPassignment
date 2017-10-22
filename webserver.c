#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <stdlib.h>
#include <errno.h>


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define As_Argument

int childsocket_no = 0;

size_t filesize;

struct confData
{
	char conftype[20];				//Initial temp variable
	int portnumber;				//For Port number
	char confVal[50];				//Initial temp variable
	char contentType[50];			//File type
	char rootPath[1024];			//Working directory
	int file_size;					//Size of file being sent

}confData;

void error_handler400(int client_sock, char command[10], char URL[1024], char HTTP_ver[32]);


void error_handler400(int client_sock, char command[10], char URL[1024], char HTTP_ver[32])
{
	char statusLine[256];
	char file_name[16];

	strcpy(file_name, "error400.html");
	FILE *fp = fopen(file_name,"w");
	int fd = open(file_name, O_RDONLY);
	off_t offset = 0;
	if(fp)
	{
		strcpy(statusLine,"HTTP/1.1 400 Bad Request\n\n");
		write(client_sock, statusLine, strlen(statusLine));

		sendfile(client_sock, fd, &offset, sizeof(statusLine));
	}
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2048], command[10], URL[1024], http_ver[32], *file_type, child_no[256], dummyVar[12], first_charec[5];
	char delim[] = " ";
	pid_t PID;
	char *linebuffer = NULL;

	FILE *file_pointer = fopen("ws.conf", "rb");


	#ifdef As_Argument
	int Port_addr = atoi(argv[1]);
	#else
	while((getline(&linebuffer, &filesize, file_pointer)) != -1)
	{
		if(strcmp(first_charec, "#") == 0)
			continue;
		strcpy(confData.conftype, strtok(linebuffer, delim));
		strcpy(confData.confVal, strtok(NULL, "\n"));
		
		if(!strcmp(confData.conftype, "Listen"))
		{
			confData.portnumber = atoi(confData.confVal);
			break;
		}
		// printf("Structure value: confData.conftype: %s ; confData.confVal: %s; confData.rootPath: '%s'; ContentType: %s\n", confData.conftype, confData.confVal, confData.rootPath, confData.contentType);
	}

	int Port_addr = confData.portnumber;
	#endif
	fclose(file_pointer);

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
				// printf("Client Message : %s\n", client_message);
			}
			printf("Child Socket: %d.\n", childsocket_no);
			bzero(command, sizeof(command));
			bzero(URL, sizeof(URL));
			bzero(http_ver, sizeof(http_ver));


			strcpy(command, strtok(client_message, delim));
			strcpy(URL, strtok(NULL, delim));
			strcpy(http_ver, strtok(NULL, "\n"));


			file_type = strchr(URL, '.');

			printf("File_Type: %s\n", file_type);

			// memcpy(command, strtok(client_message, delim), sizeof(strtok(client_message, delim)));
			// memcpy(URL, strtok(NULL, delim), sizeof(strtok(NULL, delim)));

			FILE *file_pointer = fopen("ws.conf", "rb");

			//Reading the ws.conf file
			while((getline(&linebuffer, &filesize, file_pointer)) != -1)
			{
				memcpy(first_charec, linebuffer, sizeof(char));
				if(strcmp(first_charec, "#") == 0)
					continue;
				else if(strcmp(first_charec, ".") == 0)
				{
					memcpy(confData.contentType, confData.confVal, sizeof(confData.confVal));					
				}

				// printf("linebuffer: %s\n", linebuffer);
				strcpy(confData.conftype, strtok(linebuffer, delim));
				strcpy(confData.confVal, strtok(NULL, "\n"));
				// printf("Received Data and Value are : '%s' : '%s'\n", confData.conftype, confData.confVal);

				int val = 10;

				if ((val = strcmp(confData.conftype, "DocumentRoot")) == 0)
				{
					strcpy(confData.rootPath, confData.confVal);
				}

				// printf("Structure value: confData.conftype: %s ; confData.confVal: %s; confData.rootPath: '%s'; ContentType: %s\n", confData.conftype, confData.confVal, confData.rootPath, confData.contentType);
			}
			printf("Root path: %s\n", confData.rootPath);

			// {
			// 	if(!(strcmp(http_ver, "HTTP/1.1")))
			// 	{
			// 		error_handler400(client_sock, NULL, NULL, http_ver);
			// 		// exit_gracefully();
			// 	}
			// 	else if(!strcmp(command, "GET"))
			// 	{
			// 		error_handler400(client_sock, command, NULL, NULL);
			// 	}
			// 	// else if(!)
			// }

			//printf("Command is : '%s'\n", command);
			printf("Child Socket: %d.URL is: '%s'\n", childsocket_no, URL);

			{
				char fileName[128];
				char writeBuffer[1024];	

				if(strcmp(URL, "/") == 0)
					strcpy(URL, "/index.html");
				
				strncpy(fileName, URL+1, sizeof(URL));

				strcat(confData.rootPath, URL);
				strcpy(URL, confData.rootPath);
			//	memcpy(URL, URL+1, strlen(URL));
				printf("Child Socket: %d.Opening '%s' file\n", childsocket_no, URL);
				FILE *fptr = fopen(URL, "rb");
				int fd = open(URL, O_RDONLY);
				off_t offset = 0;
				if (fptr)
				{
					printf("Child Socket: %d.File Pointer: '%p'\n", childsocket_no,fptr);
					fseek(fptr, 0, SEEK_END);
					filesize = ftell(fptr);
					fseek(fptr, 0, SEEK_SET);

					printf("Child Socket: %d.file Size: %ld bytes\n", childsocket_no, filesize);

					unsigned long int sentSize = 0;
					char statusLine[256], Content_Type[256], Content_Length[256], Connection_type[256];
					//bzero(statusLine, sizeof(statusLine));
					strcpy(statusLine,"HTTP/1.1 200 OK\n\n");
					sprintf(Content_Type,"Content-Type:%s\n", confData.contentType);
					sprintf(Content_Length,"Content-Length: %lu\n", filesize);
					sprintf(Connection_type,"Connection: keep-alive\n\n");
					int nby;
					if(nby = write(client_sock, statusLine, strlen(statusLine)) == -1)
					{
						printf("Write failed\n");
						exit(0);
					}
					// if(nby = write(client_sock, Content_Type, strlen(Content_Type)) == -1)
					// {
					// 	printf("Write failed\n");
					// 	exit(0);
					// }
					// if(nby = write(client_sock, Content_Length, strlen(Content_Length)) == -1)
					// {
					// 	printf("Write failed\n");
					// 	exit(0);
					// }
					// if(nby = write(client_sock, Connection_type, strlen(Connection_type)) == -1)
					// {
					// 	printf("Write failed\n");
					// 	exit(0);
					// }

					// printf("%s%s%s%s", statusLine, Content_Type, Content_Length, Connection_type);

					//int p=0;
					// while (sentSize < filesize)
					// {
					// 	if(sentSize == filesize) return 0;
					// 	else
					// 	{
							//if(p<10) 	printf("Child Socket: %d.WriteBuffer: '%s', file: '%s' filesize: %ld, fptr: %lu\n\n", childsocket_no, writeBuffer, URL, filesize, ftell(fptr));
						// memset(writeBuffer, '\0', sizeof(writeBuffer));
							//Receive a message from client
							//printf("Child Socket: %d.fptr position before fread: %ld\n", childsocket_no, ftell(fptr));
						// int read = fread(writeBuffer, sizeof(char), 1024, fptr);
						sendfile(client_sock, fd, &offset,filesize);
							//printf("WriteBuffer: '%s', file: '%s' filesize: %ld\n", writeBuffer, URL, filesize);
						// write(client_sock, writeBuffer, read);
						// printf("%s%s%s%s", statusLine, Content_Type, Content_Length, Connection_type);

						// printf("%s",writeBuffer);
						// sentSize += read;
							//if(p<10) printf("Child Socket: %d.sentSize: '%lu', stringlength of WriteBuffer: '%lu'\n", childsocket_no, sentSize, strlen(writeBuffer));
							//p++;
						// }
					// }
					printf("Child Socket: %d. EOF. Sent Size: %lu\n", childsocket_no, sentSize);
					fclose(file_pointer);
					fclose(fptr);				

				}
				else
				{
					int errsv = errno;
					printf("Child Socket: %d.File Pointer: '%p'. fopen failed. Error number: %d\n", childsocket_no, fptr, errsv);
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