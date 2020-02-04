/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define PORT 34906
#define BACKLOG 10		   // how many pending connections queue will hold
#define MAXDATASIZE 256	 // max length of key / value
#define MAXOPERATIONSIZE 8 // max length of operation
#define MAXPAIR 20

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char **argv)
{
	int sockfd;					   /* socket */
	socklen_t clientlen;		   /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp;		   /* client host info */
	char *hostaddrp;			   /* dotted decimal host addr string */
	int optval;					   /* flag value for setsockopt */
	int n;						   /* message byte size */
	int lastID = 0;
	char message[MAXDATASIZE * 2 + MAXOPERATIONSIZE];
	char getmessage[10];
	char send_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];

	if(argc != 2){
		fprintf(stderr, "usage: %s <possibiality to loss>\n", argv[0]);
        exit(0);
	}

	int possible = atoi(argv[1]);

	if(possible >= 100){
		fprintf(stderr, "can not have possibiality >= 100\n");
        exit(0);
	}

	/* 
	* socket: create the parent socket 
	*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
	* us rerun the server immediately after we kill it; 
	* otherwise we have to wait about 20 secs. 
	* Eliminates "ERROR on binding: Address already in use" error. 
	*/
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			   (const void *)&optval, sizeof(int));

	/*
	* build the server's Internet address
	*/
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	/* 
	* bind: associate the parent socket with a port 
	*/
	if (bind(sockfd, (struct sockaddr *)&serveraddr,
			 sizeof(serveraddr)) < 0)
		error("ERROR on binding");

	/* 
	* main loop: wait for a datagram, then echo it
	*/
	clientlen = sizeof(clientaddr);
	while (1)
	{

		/*
		* recvfrom: receive a UDP datagram from a client
		*/
		bzero(message, sizeof(message));
		bzero(getmessage, sizeof(getmessage));
		bzero(send_message, sizeof(send_message));

		recvfrom(sockfd, message, sizeof(message), 0,
							 (struct sockaddr *)&clientaddr, &clientlen);
		// int readn = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&clientaddr, &clientlen);
		// printf("receive %d byte of data\n", readn);

		/* 
		* gethostbyaddr: determine who sent the datagram
		*/
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
							  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
			error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			error("ERROR on inet_ntoa\n");
		//printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);

		char *ptr = strtok(message, "|");

		char *splitedMessage[2];
		n = 0;
		while (ptr != NULL)
		{
			splitedMessage[n++] = ptr;
			ptr = strtok(NULL, "|");
		}

		int id = atoi(splitedMessage[0]);
		
		if (id != lastID){
			printf("Error Label");
		}

		printf("receive id:%d, message:%s", id, splitedMessage[1]);


		if (strncmp("quit", splitedMessage[1], 4) == 0)
		{
			printf("Server Exit...\n");
			break;
		}

		n = 0;
		printf("enter message start with Y means receive success, else fail: ");
		while ((getmessage[n++] = getchar()) != '\n')
			;

		//printf("entered message:%s \n", getmessage);

		if (strncmp(getmessage, "Y", 1) == 0)
		{
			//int num = (rand() %  (100 - 0 + 1)) + 0;
			//if (num < possible){ 
				lastID += 1;
				snprintf(send_message, sizeof(send_message), "%d", id);
				sendto(sockfd, send_message, strlen(send_message), 0,
									(struct sockaddr *)&clientaddr, clientlen);
				//printf("send %d byte of message %s \n", writen, send_message);
			//}
		}		
	}
	return 0;
}