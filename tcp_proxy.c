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

#define PORT 34903 // the CLIENT_PORT client will be connecting to 

#define MAXDATASIZE 40 // max number of bytes of key / value
#define MAXOPERATIONSIZE 8 // max length of operation
#define MAXPAIR 20

int serversocketfd;
int clientsocketfd;

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

char *duplicateChar(char *value)
{
	char newValue[MAXDATASIZE*2];
	int j = 0;
	for(int i = 0; i < strlen(value); i ++)
	{
		newValue[j] = value[i];
		j++;
		if (value[i] == 'c' || value[i] == 'm' || value[i] == 'p' || value[i] == 't')
		{
			newValue[j] = value[i];
			j++;
		}
	}
	newValue[j] = '\0';
	char* somevalue =  (char*)malloc(MAXDATASIZE*2);
	strcpy(somevalue, newValue);
	return somevalue;
}

int generateserversocket(){
  int sockfd; 
	socklen_t len;
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
	else{
			printf("Socket successfully binded..\n"); 
	}

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
			printf("Listen failed...\n"); 
			exit(0); 
	} 
	else
			printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	serversocketfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
	if (serversocketfd < 0) { 
			printf("server acccept failed...\n"); 
			exit(0); 
	} 
	else{
			printf("server acccept the client...\n"); 
	}

	return 0;

}

int generateclientsocket(char* hostname, char* port)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((clientsocketfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(clientsocketfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(clientsocketfd);
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

	return 0;
}


void sendServerString()
{
	char send_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];
	char recv_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];

	for (;;) {
		bzero(send_message, sizeof(send_message));
		bzero(recv_message, sizeof(recv_message));

		int readn = recv(serversocketfd, recv_message, sizeof(recv_message),0);
		printf("receive %d byte of data: %s\n", readn, recv_message);
		strncpy(send_message, recv_message, strlen(recv_message));

		int writen = send(clientsocketfd, send_message, sizeof(send_message),0);
		if (writen == -1){
			printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
		}
		printf("send %d byte of data to server: %s\n", writen, send_message);
	}

}

void sendClientString(){
	char send_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];
	char recv_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];

	for (;;) {
		bzero(send_message, sizeof(send_message));
		bzero(recv_message, sizeof(recv_message));

		int readn = recv(clientsocketfd, recv_message, sizeof(recv_message),0);
		printf("receive %d byte of data: %s\n", readn, recv_message);
		if(strstr(recv_message,";") != NULL){
			char *ptr = strtok(recv_message, ";");
			while (ptr != NULL)
			{
				char* tmp = strdup(ptr);
				char *keyptr = strtok(tmp, ":");
				strcat(send_message,keyptr);
				strcat(send_message,":");
				keyptr = strtok(NULL, ":");
				char* newvalue = duplicateChar(keyptr);
				strcat(send_message,newvalue);
				strcat(send_message, ";");
				ptr = strtok(NULL,";");
			}
		}
		else{
			strncpy(send_message, recv_message, strlen(recv_message));
		}

		int writen = send(serversocketfd, send_message, sizeof(send_message),0);
		if (writen == -1){
			printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
		}
		printf("send %d byte of data to client: %s\n", writen, send_message);
		
	}

}

int main(int argc, char *argv[])
{
	if (argc != 3) {
	    fprintf(stderr,"usage: proxy server_hostname server_port");
	    exit(1);
	}  

  int serversuccess = generateserversocket();

	int clientsuccess = generateclientsocket(argv[1], argv[2]);

	printf("surver socket status %d, client socket status %d \n", serversuccess, clientsuccess);

	// Function for chatting between client and server 
	if(!fork()){
		sendServerString(); 
	}

	if(!fork()) {
		sendClientString();
	}


	// After chatting close the socket 
	close(serversocketfd);
	close(clientsocketfd); 

	return 0;
}

