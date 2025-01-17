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
#define MAXDATASIZE 40 // max length of key / value
#define MAXOPERATIONSIZE 8 // max length of operation
#define MAXPAIR 20

char *keys[MAXPAIR];
char *values[MAXPAIR];
int use[MAXPAIR]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int addKey(char* key, char* value)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (use[i] == 0)
        {
            keys[i] = (char*)malloc(strlen(key));
            values[i] = (char*)malloc(strlen(value));
            memcpy(keys[i], key, strlen(key));
            memcpy(values[i], value, strlen(value));
            use[i] = 1;
            return 1;
        }
    }
    return -1;
}

int getKey(char* key)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (use[i] == 1)
        {
					if(strncmp(key, keys[i], strlen(keys[i])) == 0){
            return i;
					}
        }
    }
    return -1;
}

int removeKey(char* key)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (strncmp(key, keys[i], strlen(key)) == 0)
        {
            use[i] = 0;
            return 1;
        }
    }
    return -1;
}

char* getAll()
{
    char *allInfo = (char*)malloc(MAXDATASIZE * MAXPAIR * 2);
    char key[MAXDATASIZE];
	char value[MAXDATASIZE * 2];
    for (int i = 0; i< MAXPAIR; i++)
    {
        if(use[i] == 1){
            bzero(key, sizeof(key)); 
		    bzero(value, sizeof(value));
            memcpy(key, keys[i], strlen(keys[i]) * sizeof(char));
            memcpy(value, values[i], strlen(values[i]) * sizeof(char));
            strcat(allInfo, key);
            strcat(allInfo,":");
            strcat(allInfo, value);
            strcat(allInfo, ";");
        }
    }
    return allInfo;
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
	char message[MAXDATASIZE * 2 + MAXOPERATIONSIZE];
	char send_message[MAXDATASIZE * MAXPAIR * 2 + MAXOPERATIONSIZE];
	int n;
    // infinite loop for chat 
    for (;;) { 
			bzero(message, sizeof(message)); 
			bzero(send_message, sizeof(send_message));

			// read the message from client and copy it in buffer 
			int readn = recv(sockfd, message, sizeof(message),0);
			printf("receive %d byte of data: %s\n", readn, message);

			char *ptr = strtok(message, "|");

			char *splitedMessage[3];
			n = 0;
			while (ptr != NULL)
			{
				splitedMessage[n++] = ptr;
				ptr = strtok(NULL,"|");
			}

			int operation_code = validOperation(splitedMessage[0]);
			int result_code = 0;
			if (operation_code == 1)
			{
				printf("Server Exit...\n"); 
				break; 
			}
			else if (operation_code == 2)
			{
				strcpy(send_message,getAll());
			}
			else if (operation_code == 3)
			{
				int get_index = getKey(splitedMessage[1]);
				printf("get index %d \n",get_index);
				if (get_index < 0){
					strcpy(send_message,"get fail");
				}
				else{
					char get_value[MAXDATASIZE];
					bzero(get_value,sizeof(get_value));
					memcpy(get_value,values[get_index], strlen(values[get_index]));
					//char *get_value = strdup(values[get_index]);
					strcpy(send_message,get_value);
				}
			}
			else if (operation_code == 4)
			{
				result_code = removeKey(splitedMessage[1]);
				if(result_code < 0){
					strcpy(send_message,"remove fail");
				}
				else{
					strcpy(send_message,"remove success");
				}
			}
			else if (operation_code == 5)
			{
				//char* newvalue = duplicateChar(splitedMessage[2]);
				printf("duplicated string: %s \n",splitedMessage[2]);
				result_code = addKey(splitedMessage[1], splitedMessage[2]);
				if(result_code < 0){
					strcpy(send_message,"add fail");
				}
				else{
					strcpy(send_message,"add success");
				}
			}
			//printf("%s",send_message);
			//int writen = write(sockfd, send_message, sizeof(send_message));
			//printf("send %d byte of data: %s \n", writen, send_message);
			int writen = send(sockfd, send_message, sizeof(send_message),0);
			printf("send %d byte of data: %s\n", writen, send_message);
    } 
}

int main(void)
{
	int sockfd, connfd; 
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

    // Function for chatting between client and server 
    sendString(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 

	return 0;
}

