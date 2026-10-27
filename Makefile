CC = gcc
CFLAGS = -Wall -Wextra -std=c11

.PHONY: all clean

all: server/server client/client

server/server: server/server.c
	$(CC) $(CFLAGS) $< -o $@

client/client: client/client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f server/server client/client server/*.o client/*.o
