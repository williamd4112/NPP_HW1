#include "netio.h"

#define SERV_PORT 8080
#define BACKLOG 10

void func_receive_file(Node *client, const char *path);

int main(int argc, char *argv[]){
	
	if(argc < 2){
		printf("usage server <Output path>");
		exit(1);
	}

	// Create Socket
	int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	// Create address struct
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPV4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // All interface
    servaddr.sin_port = htons(SERV_PORT); // Port number
    printf("Create address struct...ok\n");
    
    Bind(listenfd, (SA*)&servaddr);
	Listen(listenfd, BACKLOG);

    // Accept connection
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int clifd = accept(listenfd, (SA*)&cliaddr, &clilen);    
    if(clifd < 0){
    	perror("main: accept error\n");
    	exit(-1);
    }

    Node client;
    client.fd = clifd;
    client.addr = cliaddr;

    func_receive_file(&client, argv[1]);

    printf("Server terminated\n");
    close(listenfd);

	return 0;
}

void func_receive_file(Node *client, const char *path){
	int wrfd = open(path, O_WRONLY | O_CREAT, 0644);
	if(wrfd < 0){
		perror("func_receive_file: file open error");
		close(client->fd);
		close(wrfd);
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