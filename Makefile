CC = gcc
CLIENT_OUT = bin/client/client
CLIENT_SRC = src/client.c

SERVER_OUT = bin/server/server
SERVER_SRC = src/server.c

NETIO_SRC = src/netio.c

all:$(CLIENT_SRC) $(SERVER_SRC)
	$(CC) -D LOG -o $(CLIENT_OUT) $(CLIENT_SRC) $(NETIO_SRC)
	$(CC) -D LOG -o $(SERVER_OUT) $(SERVER_SRC) $(NETIO_SRC)
