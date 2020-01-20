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

#define PORT "34900"  // the port users will be connecting to

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

	printf("%d",strncmp(operation,add,3));

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
	
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	
	while(1) {  // main accept() loop
		char message[MAXDATASIZE * 3];
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		
		bzero(message, sizeof(message));

		read(new_fd, message, sizeof(message));

		printf("From Client: %s", message);

		char *ptr = strtok(message, "|");

		char *splitedMessage[3];
		int k = 0;

		while (ptr != NULL)
		{
			splitedMessage[k++] = ptr;
			ptr = strtok(NULL,"|");
		}

		int operation_code = validOperation(splitedMessage[0]);

		bzero(message, sizeof(message));

		strcpy(message, "Receive Message:");
		strcat(message, splitedMessage[0]);	

		if (operation_code > 2)
		{
			strcat(message, "\nKey: ");
			strcat(message, splitedMessage[1]);		
		}
		if(operation_code > 4)
		{
			strcat(message, "\nValue: ");
			strcat(message, splitedMessage[2]);	
		}

		write(sockfd, message, sizeof(message));

		if(operation_code == 1)
		{
			printf("Server Exit...\n"); 
			close(new_fd);
			return 0;
		}
		close(new_fd);  // parent doesn't need this
	}
	close(sockfd);
	return 0;
}

