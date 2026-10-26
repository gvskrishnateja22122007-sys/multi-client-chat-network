CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SERVER = server/server
CLIENT = client/client

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SERVER): server/server.c
	$(CC) $(CFLAGS) server/server.c -o $(SERVER)

$(CLIENT): client/client.c
	$(CC) $(CFLAGS) client/client.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
