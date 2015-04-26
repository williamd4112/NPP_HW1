#include "netio.h"

/*I/O Function*/
int readn(int fd, char* buf, size_t len){
    int n;
    if((n = read(fd, buf, len)) >= 0)
        buf[n] = '\0';
    return n;
}

int readline(int fd, char* buf, size_t len){
    int left,n;
    for(left = 0; left < len; left++){
        if((n = read(fd, &buf[left], 1)) < 0){
            return -1;
        }else if(buf[left] == '\n' || buf[left] == EOF){
            buf[left] = '\0';
            break;
        }
    }

    return left;
}

ssize_t writen(int fd, char* buf, ssize_t len){
    int n;
    if((n = write(fd, buf, len)) < 0)
        return -1;
    return n;
}


/*Wrapper function*/
int Socket(int domain, int type, int protocol){
    int sockfd;
    if((sockfd = socket(domain, type, protocol)) < 0){
        perror("netio: socket create error\n");
        exit(1);
    }
#ifdef LOG
    printf("Create socket...ok\n");
#endif
    return sockfd;
}

void Address(struct sockaddr_in* sock_struct, sin_family family, sin_addr addr, sin_port port){
    bzero(sock_struct, sizeof(*sock_struct));
    sock_struct->sin_family = family; 
    sock_struct->sin_addr.s_addr = addr; 
    sock_struct->sin_port = port;
#ifdef LOG
    printf("Create address struct...ok\n");
#endif
}

int Connect(int sockfd, struct sockaddr* addr){
    int flag;
    if((flag = connect(sockfd, addr, sizeof(*addr))) < 0){
        perror("netio: connect error\n");
        exit(1);
    }
#ifdef LOG
    printf("Connect...ok\n");
#endif
    return flag;
}

int Bind(int listenfd, struct sockaddr *servaddr){
    int flag;
    if((flag = bind(listenfd, servaddr, sizeof(*servaddr))) < 0){
        perror("netio: failed to bind socket\n");
        exit(1);
    }
#ifdef LOG
    printf("Bind...ok\n");
#endif
    return flag;
}

int Listen(int listenfd, int backlog){
    int flag;
    if((flag = listen(listenfd, backlog)) < 0){
        perror("netio: failed to listen\n");
        exit(1);
    }
#ifdef LOG
    printf("Listen...ok\n");
#endif
    return flag;
}

void sig_chld(int signo){
    pid_t pid;
    int stat;
    
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("Child %d terminated\n",pid);

    return;
}

void Accept(int listenfd, struct sockaddr *cliaddr, Handler handler, int option, void *args[]){
#ifdef LOG
    printf("Accepting...\n");
#endif
    while(1){
        int clilen = sizeof(*cliaddr);
        int connfd = accept(listenfd, cliaddr, &clilen);
        if(connfd < 0){
            if(errno == EINTR) continue;
            perror("netio: failed to accept\n");
        }else{
            if((option & 0x2) == DISPATCH_CHILD){
                pid_t childpid;
                if((childpid = fork()) == -1)
                    perror("netio_Accept: failed to fork\n");
                else if(childpid == 0){
                    close(listenfd);
                    handler(listenfd, connfd, cliaddr, args);
                    exit(0);
                }
            }else{
                handler(listenfd, connfd, cliaddr, args);
            }
        }

        if((option & 0x0) == ACCEPT_ONCE) break;
    }
}

fnode* FNode(fnode *node, int fd, struct sockaddr_in *addr){
    FILE *fp = fdopen(fd, "wb");
    if(fp == NULL){
        perror("netio: failed to fdopen the connfd.\n");
        close(fd);
        return NULL;
    }

    node->fp = fp;
    node->addr = *addr;

    return node;
}
