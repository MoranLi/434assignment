# Names: Yukun Li 
# NSID : yul040
# Std# : 11173592

CC = gcc
CFLAGS = -g
CPPFLAGS = -Wall -pedantic
EXTRA = -m32

.PHONY: all clean

all: clean server client tcp_server tcp_client

clean:
	rm -f *.o *.a server client tcp_server tcp_client

###################
##### PartA #######
###################

server: server.o
	$(CC) $(EXTRA) -o server server.o

server.o: server.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c server.c -o server.o

client: client.o
	$(CC) $(EXTRA) -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c client.c -o client.o
	
###################
##### PartB #######
###################

tcp_server: tcp_server.o
	$(CC) $(EXTRA) -o tcp_server tcp_server.o

tcp_server.o: tcp_server.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_server.c -o tcp_server.o

tcp_client: tcp_client.o
	$(CC) $(EXTRA) -o tcp_client tcp_client.o

tcp_client.o: tcp_client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_client.c -o tcp_client.o
