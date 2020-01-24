/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define CLIENT_PORT "34901" // the CLIENT_PORT client will be connecting to 

#define MAXDATASIZE 40 // max number of bytes of key / value
#define MAXOPERATIONSIZE 8 // max length of operation
#define MAXPAIR 20

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
	char operation[MAXOPERATIONSIZE];
	char key[MAXDATASIZE];
	char value[MAXDATASIZE];
	char message[MAXDATASIZE * 2 + MAXOPERATIONSIZE];
	char recv_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];
	int n;

	for (;;) {
		// clear buffer
		bzero(operation, sizeof(operation)); 
		bzero(key, sizeof(key)); 
		bzero(value, sizeof(value));
		bzero(message, sizeof(message));
		bzero(recv_message, sizeof(recv_message));

		// enter anc check operation
		printf("Enter the operation:");
		n = 0;
		while ((operation[n++] = getchar()) != '\n')
			; 
		
		int operation_code = validOperation(operation);

		// base on operation , determine next input
		if (operation_code < 0)
		{
			printf("error operation entered\n");
			continue;
		}
		strtok(operation, "\n");
		strcat(message, operation);
		strcat(message, "|");
		if (operation_code > 2)
		{
			printf("Enter the key:");
			n = 0;
			while ((key[n++] = getchar()) != '\n') 
				;
			strtok(key, "\n");
			strcat(message, key);
			strcat(message, "|");
			
		}
		if(operation_code > 4)
		{
			printf("Enter the value:");
			n = 0;
			while ((value[n++] = getchar()) != '\n') 
				; 
			// remove last \n 
			strtok(value, "\n");
			strcat(message, value);

		}
		strcat(message,"\n");

		int writen = send(sockfd, message, sizeof(message),0);
		printf("send %d byte of data: %s\n", writen, message);
		//bzero(message, sizeof(message));
		int readn = recv(sockfd, recv_message, sizeof(recv_message),0);
		printf("receive %d byte of data: %s\n", readn, recv_message);
		if(operation_code == 1)
		{
			printf("Client Exit...\n"); 
            break; 
		}
	}


}

int main(int argc, char *argv[])
{
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 3) {
	    fprintf(stderr,"usage: client hostname port\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	sendString(sockfd);

	close(sockfd);

	return 0;
}

