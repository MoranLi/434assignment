#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 

int recv_string(char message[], int is_client){
    int connected = 0;
    char *clientname;

    char * localmessage[900];
    memcpy(localmessage, message, strlen(message)+1);

    char *splitedMessage[28];
    int n = 0;
    char *ptr = strtok(localmessage, ";");
    while (ptr != NULL)
    {
        splitedMessage[n++] = ptr;
        ptr = strtok(NULL,";");
    }
    clientname = splitedMessage[0];
    printf("%s;\n",clientname);
    for(int i = 1;i < n; i ++)
	{	
        char *nodename = strtok(splitedMessage[i], ":");
        printf("%s:", nodename);
		char *nodedistancechar = strtok(NULL, ";");
        printf("%s;\n",nodedistancechar);
	}
    return n - 1;
}

char* send_string(char* server_name, int node_distance[], char* node_name[],  int connected, int send_to_index, int is_server){
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
    char *return_message = (char*)malloc(1024);
	strcpy(return_message, message);
    return return_message;
}


int main(){
    /*
    char mess1[]= "3;A;B:1;C:1;D:1;";
    int conn = recv_string(mess1,0);
    printf("%d\n", conn);
    char mess2[]= "A;B:1;C:1;D:1;";
    recv_string(mess2,1);*/

    char message[1024];
    int dis[] = {1,1,1,1};
    char* names[] = {"A","B","C","D"};
    char* tmp = send_string("E",dis,names,4,1,0);
    strcpy(message, tmp);
    puts(message);

}