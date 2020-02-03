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
#include <pthread.h>
#include <unistd.h>

#define BUFSIZE 1024
#define CLIENT_PORT "34901" // the CLIENT_PORT client will be connecting to

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

void* getInputThread(void *vargp)
{
    struct arg_struct *args = (struct arg_struct *)vargp;
    int *keys = args->keys;
    char **values = args->values;
    int *use = args->use;
    int *current_size = args->current_size;
    int *size = args->size;
    int *id = args->id;
    int *queue_lock = args->queue_lock;

    char entermessage[MAXDATASIZE];
    int n;
    while (1)
    {
        if (*current_size == *size)
        {
            //printf("Queue Full\n");
            continue;
        }
        else
        {
            //while (*queue_lock == 1);
            bzero(entermessage, sizeof(entermessage));
            printf("Enter the message:");
            n = 0;
            while ((entermessage[n++] = getchar()) != '\n')
                ;
            //*queue_lock = 1;
            add(keys, values, use, id, current_size, *size, entermessage);
            //*queue_lock = 0;
            if (strncmp("quit", entermessage, 4) == 0)
            {
                printf("Client Exit...\n");
                break;
            }
        }
    }

    pthread_exit(NULL);
}

void* clientThread(void *vargp)
{
    struct arg_struct *args = (struct arg_struct *)vargp;
    char* hostname = args->hostname;
    int portno = args->portno;
    int *keys = args->keys;
    char **values = args->values;
    int *use = args->use;
    int *current_size = args->current_size;
    int *size = args->size;
    int *queue_lock = args->queue_lock;

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

    int sendID = 0;
    char sendmessage[MAXDATASIZE];
    char receivemessage[MAXDATASIZE];
    char copymessage[MAXDATASIZE];

    while (1)
    {
        //while (*queue_lock == 1);
        if (*current_size == 0)
        {
            //printf("Empty queue \n");
            continue;
        }

        //*queue_lock = 1;
        bzero(sendmessage, sizeof(sendmessage));
        bzero(receivemessage, sizeof(receivemessage));
        bzero(copymessage, sizeof(copymessage));

        int index = getKey(keys, use, *size, sendID);
        if (index < 0)
        {
            *queue_lock = 0;
            continue;
        }
        memcpy(copymessage, values[index], strlen(values[index]));
        snprintf(sendmessage, sizeof(sendmessage), "%d", sendID);
        strcat(sendmessage, "|");
        strcat(sendmessage, copymessage);

        int writen = sendto(sockfd, sendmessage, sizeof(sendmessage), 0, (struct sockaddr *)&serveraddr, serverlen);
        //printf("send %d byte of data: %s\n", writen, sendmessage);

        if (strncmp("quit", copymessage, 4) == 0)
        {
            *queue_lock = 0;
            break;
        }

        int readn = recvfrom(sockfd, receivemessage, sizeof(receivemessage), 0, (struct sockaddr *)&serveraddr, &serverlen);
        //printf("receive %d byte of data: %s\n", readn, receivemessage);

        if (strncmp(receivemessage, "Y", 1) == 0)
        {
            sendID += 1;
            //removeUse(use, index);
        }
        else
        {
            sendID = atoi(receivemessage);
        }
        //*queue_lock = 0;
    }
    pthread_exit(NULL);
}

void client(char *hostname, int portno, int keys[], char *values[], int use[], int *current_size, int *size, int *queue_lock)
{
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

    int sendID = 0;
    char sendmessage[MAXDATASIZE];
    char receivemessage[MAXDATASIZE];
    char copymessage[MAXDATASIZE];

    while (1)
    {
        //while (*queue_lock == 1);
        if (*current_size == 0)
        {
            //printf("Empty queue \n");
            continue;
        }

        //*queue_lock = 1;
        printf("reach client send\n");
        bzero(sendmessage, sizeof(sendmessage));
        bzero(receivemessage, sizeof(receivemessage));
        bzero(copymessage, sizeof(copymessage));

        int index = getKey(keys, use, *size, sendID);
        if (index < 0)
        {
            *queue_lock = 0;
            continue;
        }
        memcpy(copymessage, values[index], strlen(values[index]));
        snprintf(sendmessage, sizeof(sendmessage), "%d", sendID);
        strcat(sendmessage, "|");
        strcat(sendmessage, copymessage);

        int writen = sendto(sockfd, sendmessage, sizeof(sendmessage), 0, (struct sockaddr *)&serveraddr, serverlen);
        //printf("send %d byte of data: %s\n", writen, sendmessage);

        if (strncmp("quit", copymessage, 4) == 0)
        {
            *queue_lock = 0;
            exit(0);
        }

        int readn = recvfrom(sockfd, receivemessage, sizeof(receivemessage), 0, (struct sockaddr *)&serveraddr, &serverlen);
        //printf("receive %d byte of data: %s\n", readn, receivemessage);

        if (strncmp(receivemessage, "Y", 1) == 0)
        {
            sendID += 1;
            //removeUse(use, index);
        }
        else
        {
            sendID = atoi(receivemessage);
        }
        //*queue_lock = 0;
    }
}

int main(int argc, char **argv)
{
    int portno;
    char *hostname;

    /* check command line arguments */
    if (argc != 5)
    {
        fprintf(stderr, "usage: %s <hostname> <port> <queue_size> <timeout>\n", argv[0]);
        exit(0);
    }

    hostname = argv[1];
    portno = atoi(argv[2]);
    int size = atoi(argv[3]);

    int keys[size];
    char *values[size];
    int use[size];
    int id = 0;
    int current_size = 0;
    int queue_lock = 0;

    struct arg_struct inputargs;
    inputargs.keys = keys;
    inputargs.values = values;
    inputargs.use = use;
    inputargs.size = &size;
    inputargs.current_size = &current_size;
    inputargs.queue_lock = &queue_lock;
    inputargs.id = &id;

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

    pthread_t getinputID;
    pthread_t clientID;

    pthread_create(&getinputID, NULL, getInputThread, (void*)&inputargs);
    pthread_create(&clientID, NULL, clientThread, (void*)&clientargs);

    pthread_join(getinputID, NULL);
    pthread_join(clientID,NULL);

    return 0;
}