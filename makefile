# Names: Yukun Li 
# NSID : yul040
# Std# : 11173592

CC = gcc
CFLAGS = -g
CPPFLAGS = -Wall -pedantic
PTHREAD = -lpthread

.PHONY: all clean

all: clean tcp_server

clean:
	rm -f *.o *.a tcp_server

tcp_server: tcp_server.o
	$(CC) $(EXTRA) -o tcp_server tcp_server.o $(PTHREAD)

tcp_server.o: tcp_server.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(EXTRA) -c tcp_server.c -o tcp_server.o $(PTHREAD)
