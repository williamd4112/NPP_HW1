#ifndef _NETIO_H_
#define _NETIO_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h> 
#include <arpa/inet.h>

#define SERV_PORT 8080
#define SERV_DATA_PORT 8081

#define CMDLEN 1024
#define MAXLINE 4096
#define MAXBUFF 2048
#define IPV4_ADDRLEN 16
#define ACCEPT_FOREVER 0
#define ACCEPT_ONCE 1
#define DISPATCH_CHILD 2

typedef void (*Handler) (int, int, struct sockaddr*, void *args[]);
typedef struct sockaddr SA;
typedef struct sockaddr_in SIN;
typedef short int sin_family;
typedef unsigned short sin_port;
typedef in_addr_t sin_addr; 


/* Network node , probably server or client*/
typedef struct node{
	int fd;
	struct sockaddr_in addr;
} node;

typedef struct fnode{
	FILE *fp;
	struct sockaddr_in addr;
} fnode;

// I/O Function
int readn(int, char*, size_t);
int readline(int, char*, size_t);
ssize_t writen(int, char*, ssize_t);

// Signal function
void sig_chld(int signo);

// Wrapper function
int Socket(int domain, int type, int protocol);
void Address(struct sockaddr_in* sock_struct, sin_family family, sin_addr addr, sin_port port);
int Connect(int sockfd, struct sockaddr* addr);
int Bind(int listenfd, struct sockaddr *servaddr);
int Listen(int listenfd, int backlog);
void Accept(int listenfd, struct sockaddr *cliaddr, Handler handler, int option, void *args[]);
fnode* FNode(fnode *node, int fd, struct sockaddr_in *addr);
node* Node(node *nd, int fd, struct sockaddr_in *addr);

#endif