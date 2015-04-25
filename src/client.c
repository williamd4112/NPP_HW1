#include "netio.h"

#define SERV_PORT 8080

void func_send_file(FNode *server, char *path);

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

    FNode server;
    FILE *fp = fdopen(connfd, "wb");
    if(fp == NULL){
    	perror("main: failed to fdopen the connfd.\n");
    	close(connfd);
    	exit(-1);
    }

    server.fp = fp;
    server.addr = servaddr;
    func_send_file(&server, argv[2]);
    fclose(server.fp);
	return 0;
}

void func_send_file(FNode *server, char *path){
	// int rdfd = open(path, O_RDONLY);
	// if(rdfd < 0){
	// 	perror("func_receive_file: file open error");
	// 	exit(-1);
	// }
	FILE *rdfp = fopen(path, "rb");
	if(rdfp == NULL){
		perror("func_send_file: failed to fopen\n");
		return;
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Sending file to ... %s:%d\n",
		inet_ntop(AF_INET, &server->addr.sin_addr, ip_str, sizeof(ip_str)),
		server->addr.sin_port);

	int n;
	char buff[MAXBUFF];
	while((n = fread(buff, sizeof(char), sizeof(buff), rdfp)) > 0){
		int wn = fwrite(buff, sizeof(char), n, server->fp);
		fflush(server->fp);
		if(wn < 0){
			perror("func_send_file: write error");
			fclose(rdfp);
			return;
		}else{
			printf("Write %d byte\n",wn);
		}
	}
	printf("Sending compeleted\n");
	fclose(rdfp);
}