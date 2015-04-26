#ifndef _FILETRNSFER_H_
#define _FILETRNSFER_H_

#include "netio.h"

#define DATACHANNEL_BACKLOG 10
#define VALIDCMDS_LEN 5
#define MAXFILENAME 512

// Commands
#define CD 0
#define LS 1
#define U 2
#define D 3
#define Q 4

typedef struct Client{
	struct node nd;
	int data_port;
	char cur_dir[MAXFILENAME];
}Client;

typedef struct Header{
	int type;
	char filename[MAXFILENAME];
	int filelen;
}Header;

void init_dataconn(int port, int backlog, Handler handler_datachannel, void *args[]);
void create_sender_node(node *recv_node, sin_addr *addr, int port);
void receive_file(node*, Header*, char*);
void send_file(node*, int port, char*);
void handler_datachannel(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]);
int isValid_cmd(char*);
char *fetch_cmd(char*, size_t, FILE*);
char *fetch_addr(char*);

#endif