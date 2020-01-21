/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 34901  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 40 // max number of bytes of key / value


void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int validOperation(char* operation)
{
	char add[] = "add";
	char getvalue[] = "getvalue";
	char getall[] = "getall";
	char remove[] = "remove";
	char quit[] = "quit";

	if(strncmp(operation,quit,4) == 0)
	{
		return 1;
	}
	if(strncmp(operation,getall,6) == 0)
	{
		return 2;
	}
	if(strncmp(operation,getvalue,8) == 0)
	{
		return 3;
	}
	if(strncmp(operation,remove,6) == 0)
	{
		return 4;
	}
	if(strncmp(operation,add,3) == 0)
	{
		return 5;
	}
	else
	{
		return -1;
	}
}

void sendString(int sockfd)
{
	char key[MAXDATASIZE];
	char value[MAXDATASIZE];
	char message[MAXDATASIZE * 3];
	char send_message[MAXDATASIZE * 3];
	int n;
    // infinite loop for chat 
    for (;;) { 
        bzero(key, sizeof(key)); 
		bzero(value, sizeof(value));
		bzero(message, MAXDATASIZE * 3); 
		bzero(send_message, MAXDATASIZE * 3);
  
        // read the message from client and copy it in buffer 
        read(sockfd, message, sizeof(message)); 


		char *ptr = strtok(message, "|");

		char *splitedMessage[3];
		n = 0;
		while (ptr != NULL)
		{
			splitedMessage[n++] = ptr;
			ptr = strtok(NULL,"|");
		}

		int operation_code = validOperation(splitedMessage[0]);

		strcpy(send_message, "Receive Message:");	

		if (operation_code > 2)
		{
			strcat(send_message, "\nKey: ");
			memcpy(key, splitedMessage[1], sizeof(splitedMessage[1]));
			strcat(send_message, key);		
		}
		if(operation_code > 4)
		{
			strcat(send_message, "\nValue: ");
			memcpy(value, splitedMessage[2],  sizeof(splitedMessage[2]));
			strcat(send_message, value);	
		}

		write(sockfd, send_message, sizeof(send_message));
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("quit", splitedMessage, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
}

int main(void)
{
	int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else{
        printf("server acccept the client...\n"); 
	}
	printf("message1");

    // Function for chatting between client and server 
    sendString(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 

	return 0;
}
