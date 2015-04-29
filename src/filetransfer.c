#include "filetransfer.h"

const char *VALIDCMDS[] = {"cd", "ls", "u", "d", "q"};
const char ACK = 0x1;
const char NAK = 0X0;

char output_path[MAXFILENAME] = "./";
int datachannel_fd;

int isValid_cmd(char *cmd){
	int i;
	for(i = 0; i < VALIDCMDS_LEN; i++){
		if(strncmp(cmd, VALIDCMDS[i], strlen(VALIDCMDS[i])) == 0)
			return i;
	}
	return 0;
}

void create_data_node(node *data_node, int backlog, Handler handler_datachannel, void *args[]){
    pid_t childpid;
   
   	int listenfd_data = Socket(AF_INET, SOCK_STREAM, 0);
    datachannel_fd = listenfd_data;

	//struct sockaddr_in servaddr_data;
	Address(&data_node->addr, AF_INET, htonl(INADDR_ANY), 0); // any port
    Bind(listenfd_data, (SA*)&data_node->addr);
	Listen(listenfd_data, backlog);

	socklen_t len = sizeof(data_node->addr);
	if (getsockname(listenfd_data, (SA*)&data_node->addr, &len) == -1){
    	perror("init_dataconn_client_getsockname():");
    	exit(-1);
	}

    if((childpid = fork()) == -1){
        perror("init_dataconn: failed to fork\n");
        exit(-1);
    }
    else if(childpid == 0){
    	// Accept connection
    	struct sockaddr_in cliaddr_data;
    	socklen_t clilen_data = sizeof(cliaddr_data);

    	Accept(listenfd_data, (SA*)&cliaddr_data,handler_datachannel, ACCEPT_FOREVER | DISPATCH_CHILD, args);
    	close(listenfd_data);
    	exit(0);
    }

}

pid_t init_dataconn(int port, int backlog, Handler handler_datachannel, void *args[]){
    pid_t childpid;
   
    if((childpid = fork()) == -1){
        perror("init_dataconn: failed to fork\n");
        exit(-1);
    }
    else if(childpid == 0){
    	printf("Data channel has been created on port %d\n",port);
    	int listenfd_data = Socket(AF_INET, SOCK_STREAM, 0);
    	datachannel_fd = listenfd_data;

		struct sockaddr_in servaddr_data;
		Address(&servaddr_data, AF_INET, htonl(INADDR_ANY), htons(port));
    	Bind(listenfd_data, (SA*)&servaddr_data);
		Listen(listenfd_data, backlog);

    	// Accept connection
    	struct sockaddr_in cliaddr_data;
    	socklen_t clilen_data = sizeof(cliaddr_data);

    	Accept(listenfd_data, (SA*)&cliaddr_data,handler_datachannel, ACCEPT_FOREVER | DISPATCH_CHILD, args);
    	close(listenfd_data);
    	exit(0);
    }
    return childpid;
}

/**
	Accept handler for datachannel
	used to receive data from clifd, and write data into specific path
	the path is defined in filetransfer.c
	char output_path[]
*/
void handler_datachannel(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]){
	// Create node
	close(listenfd);
	node client = {clifd, *((SIN*)cliaddr)};
	
	// Receive header
	int n;
	Header header;
	if((n = read(client.fd, &header, sizeof(header))) <= 0){
		perror("datachannel: failed to receive file header\n");
		return;
	}
	receive_file(&client, &header, output_path);
}


void receive_file(node *client, Header *header, char *path_prefix){
	char path[MAXFILENAME];
	snprintf(path, sizeof(path), "%s%s",path_prefix, header->filename);
	
	int wrfd = open(path, O_WRONLY | O_CREAT, 0644);
	if(wrfd < 0){
		perror("func_receive_file: file open error");
		close(client->fd);
		return;
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving file from %s:%d\n",
		inet_ntop(AF_INET, &client->addr.sin_addr, ip_str, sizeof(ip_str)),
		(int)client->addr.sin_port);
	printf("Name: %s\tSize: %d\n",path, header->filelen);

	int n;
	char buff[MAXBUFF];
	while((n = read(client->fd, buff, sizeof(buff))) > 0){
		int wn = write(wrfd, buff, n);
		if(wn < 0){
			perror("func_receive_file: write error\n");
			close(client->fd);
			close(wrfd);
			return;
		}else{
#ifdef LOG
			printf("Write to %d byte to file descriptor %d\n",wn, wrfd);
#endif		
		}
	}
	printf("Receiving compeleted\n");
	close(client->fd);
	close(wrfd);
}

void send_file(node *server, unsigned short port, char *path){
	node server_data;
	create_sender_node(&server_data, &server->addr.sin_addr.s_addr, port);

    printf("Ready to send data via port: %d\n",(int)server_data.addr.sin_port);

    // Read the sending file
	int rdfd = open(path, O_RDONLY);
	if(rdfd < 0){
		perror("func_send_file: failed to fopen\n");
		return;
	}

	// Get file size
	struct stat st;
	stat(path, &st);
	int size = st.st_size;

	// fill header
	Header header;
	header.type = U;
	strncpy(header.filename, path, sizeof(header.filename));
	header.filelen = size;

	int wn = write(server_data.fd, &header, sizeof(header));
	if(wn < 0){
		perror("func_send_file: write error");
		close(rdfd);
		return;
	}else{
#ifdef LOG
		printf("Write header\n");
#endif
	}

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Sending file to %s:%d\n",
		inet_ntop(AF_INET, &server_data.addr.sin_addr, ip_str, sizeof(ip_str)),
		SERV_DATA_PORT);

	int n;
	char buff[MAXBUFF];
	while((n = read(rdfd, buff, sizeof(buff))) > 0){
		int wn = write(server_data.fd, buff, n);
		if(wn < 0){
			perror("func_send_file: write error");
			close(rdfd);
			return;
		}else{
#ifdef LOG
			printf("Write %d byte\n",wn);
#endif
		}
	}

	printf("Sending compeleted\n");

	close(server_data.fd);
	close(rdfd);
}

void create_sender_node(node *sender_node, sin_addr *addr, unsigned short port){
	sender_node->fd = Socket(AF_INET, SOCK_STREAM, 0);
	Address(&sender_node->addr, AF_INET, *addr, htons(port));
    Connect(sender_node->fd, (SA*)&sender_node->addr);
}

char *fetch_cmd(char *cmd, size_t len, FILE *fp){
	char *str = fgets(cmd, len, fp); 
	if(str != NULL){
		str[strlen(str) - 1] = '\0';
	}

	return str;
}

char *fetch_addr(char *cmd){
	char *addr_start;
	for(addr_start = cmd; *addr_start != ' ' && *addr_start != '\0'; addr_start++);
	for(; *addr_start == ' ' && *addr_start != '\0'; addr_start++);
	return (*addr_start == '\0') ? NULL : addr_start;
}