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

typedef void (*Handler) (int, int, struct sockaddr*, void *args[]);
typedef struct sockaddr SA;
typedef short int sin_family;
typedef unsigned short sin_port;
typedef in_addr_t sin_addr; 


/* Network node , probably server or client*/
typedef struct Node{
	int fd;
	struct sockaddr_in addr;
} Node;

// I/O Function
int readn(int, char*, size_t);
int readline(int, char*, size_t);
ssize_t writen(int, char*, ssize_t);

// Wrapper function
int Socket(int domain, int type, int protocol);
void Address(struct sockaddr_in* sock_struct, sin_family family, sin_addr addr, sin_port port);
int Connect(int sockfd, struct sockaddr* addr);
int Bind(int listenfd, struct sockaddr *servaddr);
int Listen(int listenfd, int backlog);
void Accept(int listenfd, struct sockaddr *cliaddr, Handler handler, int option, void *args[]);

