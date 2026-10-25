CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SERVER = server/server
CLIENT = client/client

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SEREVR): server/server.c
	$(CC) $(CFLAGS) -o $(SERVER) server/server.c

$(CLIENT): client/client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client/client.c


clean:
	rm -f $(SERVER) $(CLIENT)  server/*.o client/*.o
