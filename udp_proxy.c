/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define PORT 34907 // the CLIENT_PORT client will be connecting to

#define MAXDATASIZE 256

struct arg_struct
{
    char *hostname;
    int portno;
    int *keys;
    char **values;
    int *use;
    int *current_size;
    int *size;
    int *id;
    int *queue_lock;
    int *timeout_second;
    int *send_id;
    socklen_t clientlen;           /* byte size of client's address */
    struct sockaddr_in clientaddr; /* client addr */
    int client_sockfd;
};

int add(int keys[], char *values[], int use[], int *id, int *current_size, int size, char *value)
{
    if (*current_size == size)
    {
        printf("Queue full\n");
        return -1;
    }
    for (int i = 0; i < size; i++)
    {
        if (use[i] == 0)
        {
            keys[i] = *id;
            values[i] = (char *)malloc(strlen(value));
            memcpy(values[i], value, strlen(value));
            use[i] = 1;
            *current_size += 1;
            *id += 1;
            printf("current id %d and current size %d\n", *id - 1, *current_size);
            return 0;
        }
    }
    return -1;
}

void removeUse(int use[], int index)
{
    use[index] = 0;
}

int getKey(int keys[], int use[], int size, int key)
{
    for (int i = 0; i < size; i++)
    {
        if (use[i] == 1)
        {
            if (key == keys[i])
            {
                return i;
            }
        }
    }
    return -1;
}

char *getall(int keys[], char *values[], int size, int use[])
{
    char *allInfo = (char *)malloc(MAXDATASIZE * size * 2);
    char key[10];
    char value[MAXDATASIZE];
    for (int i = 0; i < size; i++)
    {
        if (use[i] == 1)
        {
            bzero(key, sizeof(key));
            bzero(value, sizeof(value));
            memcpy(value, values[i], strlen(values[i]));
            snprintf(key, sizeof(key), "%d", keys[i]);
            strcat(allInfo, key);
            strcat(allInfo, ":");
            strcat(allInfo, value);
            strcat(allInfo, "; ");
        }
    }
    return allInfo;
}

/* 
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(0);
}

void *clientThread(void *vargp)
{
    struct arg_struct *args = (struct arg_struct *)vargp;
    char *hostname = args->hostname;
    int portno = args->portno;
    int *keys = args->keys;
    char **values = args->values;
    int *use = args->use;
    int *size = args->size;
    int *queue_lock = args->queue_lock;
    int send_id = *(args->send_id);
    socklen_t clientlen = args->clientlen; /* byte size of client's address */
    struct sockaddr_in clientaddr = args->clientaddr;
    int client_sockfd = args->client_sockfd;

    int sockfd;
    socklen_t serverlen;
    struct hostent *server;
    struct sockaddr_in serveraddr;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* send the message to the server */
    serverlen = sizeof(serveraddr);

    char sendmessage[MAXDATASIZE];
    char receivemessage[MAXDATASIZE];
    char copymessage[MAXDATASIZE];

    *queue_lock = 1;
    bzero(sendmessage, sizeof(sendmessage));
    bzero(receivemessage, sizeof(receivemessage));
    bzero(copymessage, sizeof(copymessage));

    int index = getKey(keys, use, *size, send_id);
    memcpy(copymessage, values[index], strlen(values[index]));
    strcat(sendmessage, copymessage);

    //sendto(sockfd, sendmessage, sizeof(sendmessage), 0, (struct sockaddr *)&serveraddr, serverlen);
    int writen = sendto(sockfd, sendmessage, sizeof(sendmessage), 0, (struct sockaddr *)&serveraddr, serverlen);
    printf("send %d byte of data to server: %s\n", writen, sendmessage);

    int readn = recvfrom(sockfd, receivemessage, sizeof(receivemessage), 0, (struct sockaddr *)&serveraddr, &serverlen);
    printf("receive %d byte of data to server: %s\n", readn, receivemessage);
    if (readn < 0)
    {
        pthread_t pid;
        pthread_create(&pid, NULL, clientThread, vargp);
        pthread_join(pid, NULL);
    }
    else
    {
        removeUse(use, index);
        int writen2 = sendto(client_sockfd, receivemessage, strlen(receivemessage), 0,
               (struct sockaddr *)&clientaddr, clientlen);
        printf("send %d byte of data to client: %s\n", writen2, receivemessage);
    }
    *queue_lock = 0;
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    int portno;
    char *hostname;
    int selfport;

    /* check command line arguments */
    if (argc != 5)
    {
        fprintf(stderr, "usage: %s <self port> <server hostname> <server port> <queue size>\n", argv[0]);
        exit(0);
    }

    selfport = atoi(argv[1]);
    hostname = argv[2];
    portno = atoi(argv[3]);
    int size = atoi(argv[4]);

    int keys[size];
    char *values[size];
    int use[size];
    int id = 0;
    int current_size = 0;
    int queue_lock = 0;

    struct arg_struct clientargs;
    clientargs.keys = keys;
    clientargs.values = values;
    clientargs.use = use;
    clientargs.size = &size;
    clientargs.current_size = &current_size;
    clientargs.queue_lock = &queue_lock;
    clientargs.id = &id;
    clientargs.hostname = hostname;
    clientargs.portno = portno;

    int sockfd;                    /* socket */
    struct sockaddr_in serveraddr; /* server's addr */
    struct hostent *hostp;         /* client host info */
    char *hostaddrp;               /* dotted decimal host addr string */
    int optval;                    /* flag value for setsockopt */
    char message[MAXDATASIZE];
    char send_message[MAXDATASIZE];

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
    serveraddr.sin_port = htons(selfport);

    /* 
	* bind: associate the parent socket with a port 
	*/
    if (bind(sockfd, (struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /* 
	* main loop: wait for a datagram, then echo it
	*/
    while (1)
    {

        /*
		* recvfrom: receive a UDP datagram from a client
		*/
        bzero(message, sizeof(message));
        bzero(send_message, sizeof(send_message));

        socklen_t clientlen;           /* byte size of client's address */
        struct sockaddr_in clientaddr; /* client addr */
        clientlen = sizeof(clientaddr);

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
/*
        char *ptr = strtok(message, "|");

        char *splitedMessage[2];
        n = 0;
        while (ptr != NULL)
        {
            splitedMessage[n++] = ptr;
            ptr = strtok(NULL, "|");
        }

        int getid = atoi(splitedMessage[0]);

        printf("receive getid:%d, message:%s", getid, splitedMessage[1]); */
        printf("receive message:%s", message);

        clientargs.clientlen = clientlen;
        clientargs.clientaddr = clientaddr;
        int send_id = id;
        clientargs.send_id = &send_id;
        clientargs.client_sockfd = sockfd;

        add(keys,values,use,&id,&current_size,size,message);

        pthread_t getinputID;
        pthread_create(&getinputID, NULL, clientThread, (void *)&clientargs);
        pthread_join(getinputID, NULL);
    }
    return 0;
}