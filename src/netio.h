#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAXLINE 4096
#define MAXBUFF 2048
#define IPV4_ADDRLEN 16
#define ACCEPT_FOREVER 0
#define ACCEPT_ONCE 1

typedef void (*Handler) (int, int, struct sockaddr*);
typedef struct sockaddr SA;

typedef struct Node{
	int fd;
	struct sockaddr_in addr;
} Node;

int readn(int, char*, size_t);
int readline(int, char*, size_t);
ssize_t writen(int, char*, ssize_t);

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, struct sockaddr* addr);
int Bind(int listenfd, struct sockaddr *servaddr);
int Listen(int listenfd, int backlog);
void Accept(int listenfd, struct sockaddr *cliaddr, Handler handler, int option);

