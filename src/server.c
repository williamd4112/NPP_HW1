#include "netio.h"
#include "filetransfer.h"

#define BACKLOG 10
#define MAXMSG 1024

extern char output_path[MAXFILENAME];
extern int datachannel_fd;
extern const int ACK;
extern const int NAK;

const char *SERV_OUTPUT_PATH_DEFAULT = "./upload/";

void client_receive_cmd(Client *client);
void client_receive_info(Client *client);
void client_send_info(Client *client);
void client_send_dir(node *client);
void client_send_file(Client *client, char *path);
void client_send_msg(node *client, char *msg);

void func_client_proc(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]);

int state = WAIT;
unsigned short serv_port = 0, serv_data_port;

int main(int argc, char *argv[]){
	if(argc < 2) {
		printf("usage:./server [PORT]\n");
		exit(0);
	}

	int flag = mkdir(SERV_OUTPUT_PATH_DEFAULT, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(flag == 0) {
		char cwd[MAXFILENAME];
		getcwd(cwd, sizeof(cwd));
		printf("Create Download Directory at: %s\n",cwd);
	}

	serv_port = atoi(argv[1]);


	signal(SIGCHLD, sig_chld);

	// Create Socket
	int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	flag = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int)) < 0){
		perror("Data Channel initialization error\n");
		exit(-1);
	}

	Address(&servaddr, AF_INET, htonl(INADDR_ANY), htons(serv_port));
    Bind(listenfd, (SA*)&servaddr);
	Listen(listenfd, BACKLOG);

    // Accept connection
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    void *args[] = {argv[1]};
    
    strncpy(output_path, SERV_OUTPUT_PATH_DEFAULT, sizeof(output_path));
    
    node data_node;
   	create_data_node(&data_node, DATACHANNEL_BACKLOG, handler_datachannel, args);
   	serv_data_port = ntohs(data_node.addr.sin_port);
    // init_dataconn(serv_data_port, BACKLOG, handler_datachannel, args);
    printf("Serv_Port: %u\n",serv_port);
	printf("Serv_Data_Port: %u\n",serv_data_port);
    Accept(listenfd, (SA*)&cliaddr, func_client_proc, ACCEPT_FOREVER | DISPATCH_CHILD, args);

    printf("Server terminated\n");
    close(listenfd);

	return 0;
}

void func_client_proc(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]){
    // Create Socket
    node clind = {clifd, (*(SIN*)cliaddr)};
    Client client;
    client.nd = clind;
    client.data_port = -1;
    strncpy(client.cur_dir, ".", sizeof(client.cur_dir));

    client_receive_info(&client);
    client_send_info(&client);
    client_receive_cmd(&client);
}

void client_send_dir(node *client){
	char *path = ".";
	char cwd[MAXFILENAME];
	getcwd(cwd, sizeof(cwd));

    DIR *d;
    struct dirent *dir;
    d = opendir(path);

    if(d){
        char buff[MAXLINE];
        while((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
                snprintf(buff, MAXLINE, "%s/%s\n",cwd, dir->d_name);
                
                char *relative_path = strrchr(buff, '/');
                write(client->fd, relative_path, strlen(relative_path));
            }
        }
        closedir(d);
    }   
}

void client_send_msg(node *client, char *msg){
	int wn = write(client->fd, msg, strlen(msg));
	if(wn < 0){
		perror("client_send_msg: failed to send message\n");
		return;
	}
}

void client_receive_info(Client *client){
#ifdef LOG
	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving client info from ... %s:%d\n",
		inet_ntop(AF_INET, &client->nd.addr.sin_addr, ip_str, sizeof(ip_str)),
		(int)client->nd.addr.sin_port);
#endif

	int n;
	if((n = read(client->nd.fd, &client->data_port, sizeof(client->data_port))) <= 0){
		perror("client_receive_info: failed to receive client info.\n");
		close(client->nd.fd);
		exit(1);
	}
	printf("Client data port: %hu\n",client->data_port);
}

void client_send_info(Client *client){
	int wn = write(client->nd.fd, &serv_data_port, sizeof(serv_data_port));
	if(wn <= 0) {
		perror("client_send_info: failed to send server info.\n");
		close(client->nd.fd);
		exit(1);
	}
}

void client_receive_cmd(Client *client){

	// Show connection message
	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving instruction from ... %s:%hu\n",
		inet_ntop(AF_INET, &client->nd.addr.sin_addr, ip_str, sizeof(ip_str)),
		ntohs(client->nd.addr.sin_port));
	
	// Receive command
	char cmd[CMDLEN];

	// Message buffer
	char msg_buff[MAXMSG];
	
	// Receive state
	int n;
	int state = 1; // Enable

	// Main I/O loop
	while((n = read(client->nd.fd, cmd, CMDLEN)) > 0){
		cmd[n] = '\0';
		
		char *path;
		int cmdid = isValid_cmd(cmd);

		if(cmdid >= 0) {
			switch(cmdid){
				case LS: 
					client_send_dir(&client->nd);
					break;
				case CD:
					if((path = fetch_addr(cmd)) != NULL){
						if(chdir(path) == 0){
							snprintf(msg_buff, sizeof(msg_buff), "Current Directory: %s\n",path);
							client_send_msg(&client->nd, msg_buff);
						}
						else{
							client_send_msg(&client->nd, "No such directory\n");
						}
					}
					else
						client_send_msg(&client->nd, "Usage: cd [filename]\n");
					break;
				case U:
					break;
				case D:
					if((path = fetch_addr(cmd)) != NULL)
						send_file(&client->nd, client->data_port, path);
					else
						printf("usage: u [filename]\n");
					break;
				case Q:
					printf("Client quit %s:%hu\n",ip_str, ntohs(client->nd.addr.sin_port));
					close(client->nd.fd);
					return;
				default:
					break;
			}
		}
	}

	printf("Client disconnected %s:%d\n",ip_str, (int)client->nd.addr.sin_port);
	close(client->nd.fd);
}


