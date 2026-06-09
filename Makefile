CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

.PHONY: all clean

all: server client

server: server.o csapp.o
	$(CC) -o server server.o csapp.o -lpthread

client: client.o csapp.o
	$(CC) -o client client.o csapp.o -lpthread

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c
server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f server client server.o client.o