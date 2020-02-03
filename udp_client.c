/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024
#define CLIENT_PORT "34901" // the CLIENT_PORT client will be connecting to 

#define MAXDATASIZE 256

int id = 0;
int size;
int current_size;

int add(int keys[], char *values[] , int use[], char* value)
{
  if(current_size == size){
    printf("Queue full\n");
    return -1;
  }
    for (int i = 0; i< size; i++)
    {
        if (use[i] == 0)
        {
            keys[i] = id;
            values[i] = (char*)malloc(strlen(value));
            memcpy(values[i], value, strlen(value));
            use[i] = 1;
            current_size += 1;
            id += 1;
            return 0;
        }
    }
    return -1;
}

int removeKey(int keys[], int use[], int key)
{
    for (int i = 0; i< size; i++)
    {
        if (keys[i] == key)
        {
            use[i] = 0;
            current_size -= 1;
            return 0;
        }
    }
    return -1;
}

int getKey(int keys[],  int use[], int key)
{
    for (int i = 0; i< size; i++)
    {
        if (use[i] == 1)
        {
		    if(key == keys[i]){
                return i;
			}
        }
    }
    return -1;
}

char* getall(int keys[], char *values[], int use[])
{
    char *allInfo = (char*)malloc(MAXDATASIZE * size * 2);
	char key[10];
    char value[MAXDATASIZE];
    for (int i = 0; i< size; i++)
    {
        if(use[i] == 1){
          bzero(key, sizeof(key));
          bzero(value, sizeof(value));
          memcpy(value, values[i], strlen(values[i]));
          snprintf(key, sizeof(key), "%d", keys[i]);
          strcat(allInfo, key);
          strcat(allInfo,":");
          strcat(allInfo, value);
          strcat(allInfo, "; ");
        }
    }
    return allInfo;
}

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno;
    socklen_t serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;

    /* check command line arguments */
    if (argc != 5) {
       fprintf(stderr,"usage: %s <hostname> <port> <queue_size> <timeout>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    size = atoi(argv[3]);
    
    int keys[size];
    char *values[size];
    int use[size];
    char entermessage[MAXDATASIZE];
    char sendmessage[MAXDATASIZE];
    char receivemessage[MAXDATASIZE];
    int n = 0;
    int lastSuccess = -1;
    /* send the message to the server */
    serverlen = sizeof(serveraddr);

    for (;;) {
        // clear buffer
        bzero(entermessage, sizeof(entermessage));
        bzero(sendmessage, sizeof(sendmessage));
        bzero(receivemessage, sizeof(receivemessage));

        if(lastSuccess == -1){
            // enter anc check operation
            printf("Enter the message:");
            n = 0;
            while ((entermessage[n++] = getchar()) != '\n' && n < MAXDATASIZE - 1)
                ; 
            
            add(keys, values, use, entermessage);

            snprintf(sendmessage, sizeof(sendmessage), "%d", id - 1);
            strcat(sendmessage,"|");
            strcat(sendmessage, entermessage);

        }
        else{
            snprintf(sendmessage, sizeof(sendmessage), "%d", id - 1);
            strcat(sendmessage,"|");
            strcat(sendmessage, values[lastSuccess]);
            if (lastSuccess == id - 1){
                lastSuccess = -1;
            }
            else{
                lastSuccess += 1;
            }
        }
        if (!fork()){
            int writen = sendto(sockfd, sendmessage, sizeof(sendmessage),0, (struct sockaddr *)&serveraddr, serverlen);
            printf("send %d byte of data: %s\n", writen, sendmessage);

            int readn = recvfrom(sockfd, receivemessage, sizeof(receivemessage),0, (struct sockaddr *)&serveraddr, &serverlen);
            printf("receive %d byte of data: %s\n", readn, receivemessage);

            if(strncmp(receivemessage, "Y", 1) == 0){
                lastSuccess = -1;
            }
            else{
                lastSuccess = atoi(receivemessage);
            }
        }
        if(strncmp("quit", entermessage, 4) == 0)
        {
            printf("Client Exit...\n"); 
            break; 
        }
        
    }
    return 0;
}