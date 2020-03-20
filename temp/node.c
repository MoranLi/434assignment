/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

/**
 * the server thread is blocking the whole process
 * but client can not start before server, since client is wait for server reply
 * how to solve that ?
 * */

#define MAXLINK 26


struct arg_struct
{
    char* servername;
	char **connectedNodeNames;
	int *connectedNodeDistances;
	char **connectedSocketPorts;
	int *inuse;
	int numOfClient;
	int *connectedSocketFDs;
};


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int generateserversocket(int PORT){
  	int sockfd; 
	struct sockaddr_in servaddr; 
	int on = 1;

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
			printf("socket creation failed...\n"); 
			exit(0); 
	} 
	else
			printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

   if(setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) < 0){
	   perror("setsockopt() failed");
       close(sockfd);
       exit(-1);
	}

	if(ioctl(sockfd, FIONBIO, (char *)&on) < 0){
	    perror("ioctl() failed");
      close(sockfd);
      exit(-1);
	}

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
	
	return sockfd;

}

int generateclientsocket(char* hostname, char* port, int index)
{
	printf("create client socket %s, %s\n", hostname, port);
	int clientsocketFD;
	inuse[index] = 1;
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
		if ((clientsocketFD = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(clientsocketFD, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(clientsocketFD);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s:%s\n", s, port);

	freeaddrinfo(servinfo); // all done with this structure

	return clientsocketFD;
}

void* sendString(void* vargp){
	printf("send thread\n");
	struct arg_struct *args = (struct arg_struct *)vargp;
	char **connectedNodeNames = args->connectedNodeNames;
	int* connectedNodeDistances = args->connectedNodeDistances;
	char** connectedSocketPorts = args->connectedSocketPorts;
	int* inuse = args->inuse;
	char *servername = args->servername;
	int numOfClient = args->numOfClient;
	int* connectedSocketFDs = args->connectedSocketFDs;
	strcat


	//for(;;){
		//char send_message[2 + (MAXLINK-1) * 4 + MAXLINK - 2];
		//strcat(send_message, servername);
		//strcat(send_message,";");
		//printf("enter generate message %s\n", send_message);
		/*
		for (int j = 0; j < MAXLINK; j++){
			if (inuse[j] == 0){
				continue;
			}
			strcat(send_message,connectedNodeNames[j]);
			strcat(send_message, ";");
			char disstr[2];
			sprintf(disstr, "%d", connectedNodeDistances[j]);
			strcat(send_message,disstr);
		}*/
		for (int i = 0; i < numOfClient; i ++){
			// skip if this index is not in use(no data stored)
			printf("try to send to FD %d\n",connectedSocketFDs[i]);
			int writen = send(connectedSocketFDs[i], connectedSocketPorts[i], sizeof(connectedSocketPorts[i]),0);
			if (writen == -1){
				printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
			}
			printf("send %d byte of data to server: %s\n", writen, connectedSocketPorts[i]);
		}
		
	//}
	exit(0);
}

void recvString(int serversocketfd){


	// assume distance between two port is not larger than 100, so can represent in 2 char
	// message will in format of servername;clientname:distance;...;
	
	char recv_message[2 + MAXLINK *3];
	bzero(recv_message, sizeof(recv_message));

	int readn = recv(serversocketfd, recv_message, sizeof(recv_message),0);
	printf("node receive %d byte of data from server: %s\n", readn, recv_message);
	/*
	char *ptr = strtok(recv_message, ";");
	char *clientname = strdup(ptr);
	int currdistance = 0;
	int found = 0;
	for (int i = 0; i < MAXLINK; i++){
		if (inuse[i] == 1 && strstr(clientname, connectedNodeNames[i]) != NULL){
			currdistance = connectedNodeDistances[i];
			found = 1;
			break;
		}
	}
	if (found == 0){
		for (int i = 0; i < MAXLINK; i++){
			if (inuse[i] == 0){
				connectedNodeDistances[i] = 1;
				strcpy(connectedNodeNames[i], clientname);
				inuse[i] = 1;
				break;
			}
		}
	}				
	while (ptr != NULL)
	{
		char* tmp = strdup(ptr);
		char *nodename = strtok(tmp, ":");
		char *nodedistancechar = strtok(NULL, ":");
		int nodedistance = atoi(nodedistancechar) + currdistance;
		int foundt = 0;
		for (int i = 0; i < MAXLINK; i++){
			if (inuse[i] == 0){
				continue;
			}
			if (strcpy(nodename, connectedNodeNames[i]) != NULL){
				if(connectedNodeDistances[i] > nodedistance){
					connectedNodeDistances[i] = nodedistance;
				}
				foundt = 1;
				break;
			}
		}
		if (foundt == 0){
			for (int i = 0; i < MAXLINK; i++){
				if (inuse[i] == 0){
					strcpy(connectedNodeNames[i], nodename);
					connectedNodeDistances[i] = nodedistance;
					inuse[i] = 1;
					break;
				}
			}
		}
		ptr = strtok(NULL,";");
	}
	*/
}


void nodeProcess(char * servername, int serverport, char** clientports, int numOfClient ){

	
	char* servername;

	char *connectedNodeNames[MAXLINK];
	int connectedNodeDistances[MAXLINK];
	char* connectedSocketPorts[MAXLINK];
	int inuse[MAXLINK];
	fd_set readfds;

	int socketfd = generateserversocket(serverport);
	printf("server socket at %d success\n",serverport);

	for(int i = 0; i < numOfClient; i++){
		connectedNodeDistances[i] = 1;
		connectedSocketPorts[i] = clientports[i];
	}

	int connectedSocketFDs[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

	int connectedNum = 0;

	
	while(connectedNum < numOfClient){
		for(int i = 0; i < numOfClient; i++){
			connectedSocketFDs[i] = generateclientsocket("localhost", connectedSocketPorts[i], i);
			if(connectedSocketFDs[i] > 0){
				printf("connected to server socket with FD %d",connectedSocketFDs[i]);
				inuse[i] = 0;
				connectedNum += 1;
			}
		}
	}

	struct arg_struct clientargs;
    clientargs.connectedNodeDistances = connectedNodeDistances;
	clientargs.connectedNodeNames = connectedNodeNames;
	clientargs.connectedSocketPorts = clientports;
	clientargs.inuse = inuse;
	clientargs.servername = strdup(servername);
	clientargs.numOfClient = numOfClient;
	clientargs.connectedSocketFDs = connectedSocketFDs;


	for(;;){
		int received = 0;

		FD_ZERO(&readfds);

		struct timeval tv;

		tv.tv_sec = 20;
		tv.tv_usec = 0;
		int rv = select(numOfClient, &readfds, NULL, NULL, &tv);

		FD_SET(socketfd,&readfds);
		if (rv == -1) {
			perror("select"); 
		} /*else if (rv == 0) {
			printf("Timeout occurred!  No data after 10.5 seconds.\n");
		}*/
		else {
			for (int i =0; i< numOfClient; i++){
				int writen = send(connectedSocketFDs[i], connectedSocketPorts[i], sizeof(connectedSocketPorts[i]),0);
				if (writen == -1){
					printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
				}
				printf("send %d byte of data to server: %s\n", writen, connectedSocketPorts[i]);
			}
			for(int i = 0; i <= socketfd; i++) {
				if(FD_ISSET(i, &readfds)){
					printf("accept socket %d\n",i);
					if(i == socketfd){
						struct sockaddr_in cli;
						socklen_t len = sizeof(cli);
						// Accept the data packet from client and verification 
						int serversocketfd = accept(socketfd, (struct sockaddr*)&cli, &len); 
						if (serversocketfd < 0) { 
							printf("server acccept failed...\n"); 
						} 
						else{
							printf("server %d acccept the client ...\n,",serverport); 
						}
						printf("new server socket fd %d\n", serversocketfd);
						FD_SET(serversocketfd, &readfds);
						if (serversocketfd > socketfd){
							socketfd = serversocketfd;
						}
						recvString(serversocketfd);
						received += 1;
					}
					else{
						printf("read exist connect %d\n",i);
						recvString(i);
						received += 1;
					}
				}
			}
		}
			
	}
}


int main(int argc, char *argv[])
{
	if(!fork()){
		char *otherNodes[] = {"30002"};
		nodeProcess("a",30003, otherNodes, 1);
	}
	if(!fork()){
		char *otherNodes[] = {"30003"};
		nodeProcess("b",30002, otherNodes, 1);
	}
}

