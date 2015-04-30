CC = gcc
CLIENT_OUT = bin/client/HW1_102062171_Cli 
CLIENT_SRC = src/client.c

SERVER_OUT = bin/server/HW1_102062171_Ser
SERVER_SRC = src/server.c 

NETIO_SRC = src/netio.c src/filetransfer.c

all:$(CLIENT_SRC) $(SERVER_SRC)
	$(CC) -o $(CLIENT_OUT) $(CLIENT_SRC) $(NETIO_SRC)
	$(CC) -o $(SERVER_OUT) $(SERVER_SRC) $(NETIO_SRC)

log:$(CLIENT_SRC) $(SERVER_SRC)
	$(CC) -D LOG -o $(CLIENT_OUT) $(CLIENT_SRC) $(NETIO_SRC)
	$(CC) -D LOG -o $(SERVER_OUT) $(SERVER_SRC) $(NETIO_SRC)

clean:
	rm -f bin/client/client
	rm -f bin/server/server
