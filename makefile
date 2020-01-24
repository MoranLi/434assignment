# Names: Yukun Li 
# NSID : yul040
# Std# : 11173592

CC = gcc
CFLAGS = -g
CPPFLAGS = -Wall -pedantic
#EXTRA = -m32

.PHONY: all clean

all: clean tcp_server tcp_client

p1: tcp_client tcp_server

p2: tcp_client tcp_server tcp_proxy

clean:
	rm -f *.o *.a tcp_server tcp_client tcp_rpoxy

tcp_server: tcp_server.o
	$(CC) $(EXTRA) -o tcp_server tcp_server.o

tcp_server.o: tcp_server.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_server.c -o tcp_server.o

tcp_client: tcp_client.o
	$(CC) $(EXTRA) -o tcp_client tcp_client.o

tcp_client.o: tcp_client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_client.c -o tcp_client.o


tcp_proxy: tcp_proxy.o
	$(CC) $(EXTRA) -o tcp_proxy tcp_proxy.o

tcp_proxy.o: tcp_proxy.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_proxy.c -o tcp_proxy.o

