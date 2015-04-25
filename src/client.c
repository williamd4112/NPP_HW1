#include "netio.h"

#define SERV_PORT 8080

void func_send_file(Node *server, char *path);

int main(int argc, char *argv[]){
	
	if(argc < 3){
		printf("usage client <IP Address> <Send file name>\n");
		exit(-1);
	}

	printf("Client started\n");

	// Create Socket
	int connfd = Socket(AF_INET, SOCK_STREAM, 0);

	// Address Transformation
	sin_addr addr;
	inet_pton(AF_INET, argv[1], &addr);

	struct sockaddr_in servaddr;
	Address(&servaddr, AF_INET, addr, htons(SERV_PORT));

    Connect(connfd, (SA*)&servaddr);

    Node server;
    server.fd = connfd;
    server.addr = servaddr;
    func_send_file(&server, argv[2]);

	return 0;
}

void func_send_file(Node *server, char *path){
	int rdfd = open(path, O_RDONLY);
	if(rdfd < 0){
		perror("func_receive_file: file open error");
		exit(-1);
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Sending file to ... %s\n",
		inet_ntop(AF_INET, &server->addr.sin_addr, ip_str, sizeof(ip_str)));

	int n;
	char buff[MAXBUFF];
	while((n = read(rdfd, buff, sizeof(buff))) > 0){
		int wn = write(server->fd, buff, n);
		if(wn < 0){
			perror("func_send_file: write error");
			close(server->fd);
			close(rdfd);
			return;
		}else{
			printf("Write to %d byte to file descriptor %d\n",wn, server->fd);
		}
	}
	printf("Sending compeleted\n");
	close(server->fd);
	close(rdfd);
}