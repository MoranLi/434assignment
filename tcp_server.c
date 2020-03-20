#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#define MAXLINE 1024 

char* send_string(char* server_name, int* node_distance, char* node_name[],  int connected, int send_to_index, int is_server){
    char message[1024];
    bzero(message, sizeof(message));
    strcat(message, server_name);
    strcat(message, ";");
    for(int i = 0; i<connected; i++){
        if(i == send_to_index && is_server == 1){
            continue;
        }
        strcat(message, node_name[i]);
        strcat(message, ":");
        char distance_buffer[3];
        sprintf(distance_buffer,"%d",node_distance[i]);
        strcat(message, distance_buffer);
        strcat(message, ";");
    }
    message[1023] = '\0';
    char *return_message = (char*)malloc(1024);
	strcpy(return_message, message);
    return return_message;
}

int recv_string(char* message, int node_distance[], char* node_name[], int connected){
    char *clientname;
    char *splitedMessage[28];
    int n = 0;
    char *ptr = strtok(message, ";");
    while (ptr != NULL)
    {
        splitedMessage[n++] = ptr;
        ptr = strtok(NULL,";");
    }
    clientname = splitedMessage[0];
	int currdistance = 0;
	for (int i = 0; i < connected; i++){
		if (strstr(clientname, node_name[i]) != NULL){
			currdistance = node_distance[i];
			break;
		}
	}			
	for (int i = 1; i < n; i ++)
	{
		char *nodename = strtok(splitedMessage[i], ":");
		char *nodedistancechar = strtok(NULL, ":");
		int nodedistance = atoi(nodedistancechar) + currdistance;
		int foundt = 0;
		for (int i = 0; i < n; i++){
			if (strcpy(nodename, node_name[i]) != NULL){
				if(node_distance[i] > nodedistance){
					node_distance[i] = nodedistance;
				}
				foundt = 1;
				break;
			}
		}
		if (foundt == 0){
            strcpy(node_name[connected], nodename);
			node_distance[connected] = nodedistance;
            connected += 1;
		}
		ptr = strtok(NULL,";");
	}
    if (connected < n - 1){
        return n - 1;
    }
    return connected;
}

char* client(int port, char* send_message, char* servername) 
{ 
	int sockfd; 
	struct sockaddr_in servaddr; 
    char send_string[1024];
    char recv_string[1024];

	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("socket creation failed"); 
		exit(0); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 

	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(sockfd, (struct sockaddr*)&servaddr, 
							sizeof(servaddr)) < 0) { 
		printf("\n Error : Connect Failed \n"); 
	} 

	memset(send_string, 0, sizeof(send_string)); 
    memset(recv_string, 0, sizeof(recv_string)); 
    strcpy(send_string, send_message);
	write(sockfd, send_string, strlen(send_string)+1); 
	//printf("Message from server BY client %s:", servername); 
	read(sockfd, recv_string, sizeof(recv_string)); 
	puts(recv_string); 
	close(sockfd); 
    if(strncmp(send_message,"?",1) == 0){
        char * tmp = (char*)malloc(1024);
	    strcpy(tmp, recv_string);
        return tmp;
    }
    else{
        return NULL;
    }
} 

void server(int port, int num_client, char* node_name[], char* server_name) 
{ 
    int listenfd, connfd, maxfdp1; 
    char buffer[MAXLINE]; 
    pid_t childpid; 
    fd_set rset; 
    socklen_t len; 
    struct sockaddr_in cliaddr, servaddr; 
    char* message = "\0"; 
    void sig_chld(int); 

    int connected = num_client;
    int node_distance[26];
    char send_message[1024];
    for(int i = 0; i < connected; i++){
        node_distance[i] = 1;
    }
  
    /* create listening TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0); 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
  
    // binding server addr structure to listenfd 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    listen(listenfd, 10); 
  
    // clear the descriptor set 
    FD_ZERO(&rset); 
  
    // get maxfd 
    maxfdp1 = listenfd + 1; 
    for (;;) { 
  
        // set listenfd and udpfd in readset 
        FD_SET(listenfd, &rset); 

        // select the ready descriptor 
        select(maxfdp1, &rset, NULL, NULL, NULL); 
  
        // if tcp socket is readable then handle 
        // it by accepting the connection 
        if (FD_ISSET(listenfd, &rset)) { 
            len = sizeof(cliaddr); 
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
            if ((childpid = fork()) == 0) { 
                close(listenfd); 
                bzero(buffer, sizeof(buffer)); 
                bzero(send_message, sizeof(send_message)); 
                //printf("Message From TCP client BY server %s: ", server_name); 
                read(connfd, buffer, sizeof(buffer)); 
                //puts(buffer); 
                if(strncmp(buffer,"?",1) == 0){
                    //puts("receive client request for data");
                    char * tmp = send_string(server_name,node_distance,node_name,connected, 0, 0);
                    strcpy(send_message,tmp);
                    strcat(send_message,"\0");
                    free(tmp);
                    //puts(send_message);
                    write(connfd, (const char*)send_message, strlen(send_message)+1); 
                }
                else{
                    connected = recv_string(buffer,node_distance,node_name, connected);
                    write(connfd, (const char*)message, strlen(message)+1); 
                }
                close(connfd); 
                exit(0); 
            } 
            close(connfd); 
        } 
    } 
}

void router(int server_port, int client_ports[], char* node_name[], int num_client, char * server_name){
    if(!fork()){
        server(server_port, num_client, node_name, server_name);
    }
    int node_distance[num_client];
    char send_message[1024];
    char recv_message[2048];
    int connected = num_client;
    for(int i = 0; i < num_client; i++){
        node_distance[i] = 1;
    }
    for(;;){
        sleep(2);
        for(int i = 0; i < num_client; i++){
            bzero(send_message,sizeof(send_message));
            char* tmp = send_string(server_name,node_distance,node_name,connected, i, 1);
            strcpy(send_message, tmp);
            strcat(send_message,"\0");
            client(client_ports[i],send_message, server_name);
        }
        bzero(recv_message,sizeof(recv_message));
        char * tmp = client(server_port, "?",server_name);
        connected = recv_string(tmp,node_distance, node_name, connected);
        puts(recv_message);
    }
}

int main(){
    if(!fork()){
        int clientports [] = {30007,30008}; 
        char *clientname [] = {"B", "C"};
        router(30009,clientports, clientname, 2,"A");
    }
    if(!fork()){
        int clientports [] = {30009,30008};
        char *clientname [] = {"A", "C"};
        router(30007,clientports, clientname, 2,"B");
    }
    int clientports [] = {30007,30009};
    char *clientname [] = { "B","A"};
    router(30008,clientports, clientname, 2,"C");
}