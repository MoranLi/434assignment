#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

#define MAXLINK 26

struct arg_struct
{
   char *servername;
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
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in *)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int generateclientsocket(char *hostname, int port, int index)
{
   printf("create client socket %s, %d\n", hostname, port);
   int clientsocketFD;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   char s[INET6_ADDRSTRLEN];

   char portstr[5];
   sprintf(portstr, "%d", port);

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   if ((rv = getaddrinfo(hostname, portstr, &hints, &servinfo)) != 0)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and connect to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((clientsocketFD = socket(p->ai_family, p->ai_socktype,
                                   p->ai_protocol)) == -1)
      {
         perror("client: socket");
         continue;
      }

      if (connect(clientsocketFD, p->ai_addr, p->ai_addrlen) == -1)
      {
         perror("client: connect");
         close(clientsocketFD);
         continue;
      }

      break;
   }

   if (p == NULL)
   {
      fprintf(stderr, "client: failed to connect\n");
      return -1;
   }

   inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
             s, sizeof s);
   printf("client: connecting to %s:%d\n", s, port);

   freeaddrinfo(servinfo); // all done with this structure

   return clientsocketFD;
}

void sendString()
{
   /*
	printf("send thread\n");
	struct arg_struct *args = (struct arg_struct *)vargp;
	char** connectedSocketPorts = args->connectedSocketPorts;
	int numOfClient = args->numOfClient;
	int* connectedSocketFDs = args->connectedSocketFDs;
   */
   int clientsocketfd = -1;

   while (clientsocketfd < 0)
   {
      sleep(2);
      clientsocketfd = generateclientsocket("localhost", 12345, 0);
   }

   for (;;)
   {
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
      sleep(2);
      //for (int i = 0; i < numOfClient; i ++){
      // skip if this index is not in use(no data stored)
      //printf("try to send to FD %d\n",clientsocketfd);
      char portstr[5];
      sprintf(portstr, "%d", 12345);
      int writen = send(clientsocketfd, portstr, sizeof(portstr), MSG_NOSIGNAL);
      if (writen == -1)
      {
         printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
      }
      //printf("send %d byte of data to server: %s\n", writen, portstr);
      //}
   }
   pthread_exit(NULL);
}

void *sendStringThread(void *vargp)
{
   printf("send thread\n");
   struct arg_struct *args = (struct arg_struct *)vargp;
   char **connectedNodeNames = args->connectedNodeNames;
   int *connectedNodeDistances = args->connectedNodeDistances;
   char **connectedSocketPorts = args->connectedSocketPorts;
   int *inuse = args->inuse;
   char *servername = args->servername;
   int numOfClient = args->numOfClient;
   int *connectedSocketFDs = args->connectedSocketFDs;

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
   for (;;)
   {
      sleep(5);
      for (int i = 0; i < numOfClient; i++)
      {
         // skip if this index is not in use(no data stored)
         printf("try to send to FD %d\n", connectedSocketFDs[i]);
         int writen = send(connectedSocketFDs[i], connectedSocketPorts[i], sizeof(connectedSocketPorts[i]), MSG_NOSIGNAL);
         if (writen == -1)
         {
            printf("Oh dear, something went wrong with send()! %s\n", strerror(errno));
         }
         printf("send %d byte of data to server: %s\n", writen, connectedSocketPorts[i]);
      }
   }

   //}
   pthread_exit(NULL);
}

void router(int server_port, int client_ports[], char *servername, int num_client)
{

   char *connectedNodeNames[MAXLINK];
   int connectedNodeDistances[MAXLINK];
   char *connectedSocketPorts[MAXLINK];
   int inuse[MAXLINK];

   /*
   prepare client
   */
   int connectedSocketFDs[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

   int connectedNum = 0;

   struct arg_struct clientargs;
   clientargs.connectedNodeDistances = connectedNodeDistances;
   clientargs.connectedNodeNames = connectedNodeNames;
   clientargs.connectedSocketPorts = client_ports;
   clientargs.inuse = inuse;
   clientargs.servername = strdup(servername);
   clientargs.numOfClient = num_client;
   clientargs.connectedSocketFDs = connectedSocketFDs;

   /*
      construct server
   */
   int i, len, rc, on = 1;
   int listen_sd, max_sd, new_sd;
   int desc_ready, end_server = FALSE;
   int close_conn;
   char buffer[80];
   struct sockaddr_in6 addr;
   struct timeval timeout;
   fd_set master_set, working_set;

   listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }

   rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }

   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin6_family = AF_INET6;
   memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
   addr.sin6_port = htons(server_port);
   rc = bind(listen_sd,
             (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }

   rc = listen(listen_sd, 32);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }

   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);

   timeout.tv_sec = 20;
   timeout.tv_usec = 0;

   /*
   start clients
   */
  while (connectedNum < num_client)
   {
      for (int i = 0; i < num_client; i++)
      {
         connectedSocketFDs[i] = generateclientsocket("127.0.0.1", client_ports[i], i);
         if (connectedSocketFDs[i] > 0)
         {
            printf("connected to server socket with FD %d", connectedSocketFDs[i]);
            inuse[i] = 0;
            connectedNum += 1;
         }
      }
   }
   if(!fork()){
      /*
         pthread_t pid;
      pthread_create(&pid, NULL, sendStringThread, (void *)&clientargs);
      pthread_join(pid, NULL);
      */
     sendStringThread((void *)&clientargs);
      exit(0);
   }
   
   /*
   server listen accept loop
   */
   do
   {
      memcpy(&working_set, &master_set, sizeof(master_set));
      printf("Waiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }
      if (rc == 0)
      {
         printf("  select() timed out.  End program.\n");
         break;
      }
      desc_ready = rc;
      for (i = 0; i <= max_sd && desc_ready > 0; ++i)
      {
         if (FD_ISSET(i, &working_set))
         {
            desc_ready -= 1;
            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
               do
               {
                  new_sd = accept(listen_sd, NULL, NULL);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }
                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;
               } while (new_sd != -1);
            }

            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               do
               {
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }
                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
                     break;
                  }
                  len = rc;
                  printf("  %d bytes received\n", len);
                  printf("received data: %s", buffer);
               } while (TRUE);
               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
            } /* End of existing connection is readable */
         }    /* End of if (FD_ISSET(i, &working_set)) */
      }       /* End of loop through selectable descriptors */

   } while (end_server == FALSE);
   for (i = 0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
}

int main(int argc, char* argv[]){
   if(!fork()){
		int otherNodes[] = {30004};
		router(30005, otherNodes, "a",1);
	}
	if(!fork()){
		int *otherNodes[] = {30005};
		router(30004, otherNodes, "b",1);
	}
}