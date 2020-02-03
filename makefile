# Names: Yukun Li 
# NSID : yul040
# Std# : 11173592

CC = gcc
CFLAGS = -g
CPPFLAGS = -Wall -pedantic
PTHREAD = -lpthread

.PHONY: all clean

all: clean udp_proxy udp_server udp_client

p1: udp_server udp_client

p2: udp_server udp_proxy udp_client


clean:
	rm -f *.o *.a udp_server udp_proxy udp_client queue

udp_server: udp_server.o
	$(CC) -o udp_server udp_server.o

udp_server.o: udp_server.c
	$(CC) $(CFLAGS) $(CPPFLAGS)  -c udp_server.c -o udp_server.o

udp_client: udp_client.o
	$(CC) -o udp_client udp_client.o $(PTHREAD)

udp_client.o: udp_client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c udp_client.c -o udp_client.o $(PTHREAD)

udp_proxy: udp_proxy.o
	$(CC) -o udp_proxy udp_proxy.o

udp_proxy.o: udp_proxy.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c udp_proxy.c -o udp_proxy.o

queue: queue.o
	$(CC) -o queue queue.o

queue.o: queue.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c queue.c -o queue.o
