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

// State
#define WAIT 0
#define WAIT_CD_ACK 1
#define WAIT_LS_ACK 2
#define WAIT_U_ACK 3
#define WAIT_D_ACK 4

typedef struct Client{
	struct node nd;
	unsigned short data_port;
	char cur_dir[MAXFILENAME];
}Client;

typedef struct Header{
	char type;
	char filename[MAXFILENAME];
	int filelen;
}Header;

pid_t init_dataconn(int port, int backlog, Handler handler_datachannel, void *args[]);
void create_sender_node(node *recv_node, sin_addr *addr, unsigned short port);
void create_data_node(node *data_node, int backlog, Handler handler_datachannel, void *args[]);
void receive_file(node*, Header*, char*);
void send_file(node*, unsigned short port, char*);
void handler_datachannel(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]);
int isValid_cmd(char*);
char *fetch_cmd(char*, size_t, FILE*);
char *fetch_addr(char*);

#endif