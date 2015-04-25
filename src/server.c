#include "netio.h"

#define SERV_PORT 8080
#define BACKLOG 10

void client_receive_file(Node *client, const char *path);
void client_receive_file_fp(Node *client, const char *path);
void func_client_proc(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]);

int main(int argc, char *argv[]){
	
	if(argc < 2){
		printf("usage server <Output path>");
		exit(1);
	}

	// Create Socket
	int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	Address(&servaddr, AF_INET, htonl(INADDR_ANY), htons(SERV_PORT));
    Bind(listenfd, (SA*)&servaddr);
	Listen(listenfd, BACKLOG);

    // Accept connection
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    void *args[] = {argv[1]};
    Accept(listenfd, (SA*)&cliaddr, func_client_proc, ACCEPT_ONCE, args);

    printf("Server terminated\n");
    close(listenfd);

	return 0;
}


void func_client_proc(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]){
	char *path = (char*)args[0];

	Node client;
    client.fd = clifd;
    client.addr = *((struct sockaddr_in*)cliaddr);

    client_receive_file(&client, path);
}

void client_receive_file_fp(Node *client, const char *path){
	FILE *wrfp = fopen(path, "wb");
	if(wrfp == NULL){
		perror("func_receive_file: file open error");
		close(client->fd);
		return;
	}

	FILE *clifp = fdopen(client->fd, "rb");
	if(clifp == NULL){
		perror("func_receive_file: file open error");
		close(client->fd);
		return;
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving file from ... %s:%d\n",
		inet_ntop(AF_INET, &client->addr.sin_addr, ip_str, sizeof(ip_str)),
		(int)client->addr.sin_port);

	int n;
	char buff[MAXBUFF];
	while((n = fread(buff, sizeof(char), sizeof(buff), clifp)) > 0){
		int wn = fwrite(buff, sizeof(char), n, wrfp);
		fflush(wrfp);
		if(wn < 0){
			perror("func_receive_file: write error\n");
			fclose(clifp);
			fclose(wrfp);
			return;
		}else{
			printf("Write to %d byte to file descriptor %d\n",wn, fileno(wrfp));
		}
	}
	printf("Receiving compeleted\n");
	fclose(clifp);
	fclose(wrfp);
}

void client_receive_file(Node *client, const char *path){
	int wrfd = open(path, O_WRONLY | O_CREAT, 0644);
	if(wrfd < 0){
		perror("func_receive_file: file open error");
		close(client->fd);
		return;
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving file from ... %s\n",
		inet_ntop(AF_INET, &client->addr.sin_addr, ip_str, sizeof(ip_str)));
	
	int n;
	char buff[MAXBUFF];
	while((n = read(client->fd, buff, sizeof(buff))) > 0){
		int wn = write(wrfd, buff, n);
		if(wn < 0){
			perror("func_receive_file: write error\n");
			close(client->fd);
			close(wrfd);
			return;
		} else{
			printf("Write to %d byte to file descriptor %d\n",wn, wrfd);
		}
	}
	printf("Receiving compeleted\n");
	close(client->fd);
	close(wrfd);
}