#include "netio.h"
#include "filetransfer.h"

#define BACKLOG 10

extern char output_path[MAXFILENAME];
const char *SERV_OUTPUT_PATH_DEFAULT = "./upload/";

void client_receive_cmd(Client *client);
void client_receive_info(Client *client);
void client_send_dir(node *client);
void client_send_file(Client *client, char *path);

void func_client_proc(int listenfd, int clifd, struct sockaddr *cliaddr, void *args[]);

int main(int argc, char *argv[]){
	signal(SIGCHLD, sig_chld);

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
    
    strncpy(output_path, SERV_OUTPUT_PATH_DEFAULT, sizeof(output_path));
    init_dataconn(SERV_DATA_PORT, BACKLOG, handler_datachannel, args);
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
    client_receive_cmd(&client);
}

void client_send_dir(node *client){
    char *path = ".";
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if(d){
        char buff[MAXLINE];
        while((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
                snprintf(buff, MAXLINE, "%s/%s\n",path, dir->d_name);
                write(client->fd, buff, strlen(buff));
            }
        }
        closedir(d);
    }   
}


void client_receive_info(Client *client){
	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving client info from ... %s:%d\n",
		inet_ntop(AF_INET, &client->nd.addr.sin_addr, ip_str, sizeof(ip_str)),
		(int)client->nd.addr.sin_port);

	int clidata_port;
	int n;
	if((n = read(client->nd.fd, &clidata_port, sizeof(&clidata_port))) <= 0){
		perror("client_receive_info: failed to receive client info.\n");
		close(client->nd.fd);
		exit(1);
	}
	printf("Client data port : %d\n",clidata_port);
	client->data_port = clidata_port;
}

void client_receive_cmd(Client *client){

	char ip_str[IPV4_ADDRLEN + 1];
	printf("Receiving instruction from ... %s:%d\n",
		inet_ntop(AF_INET, &client->nd.addr.sin_addr, ip_str, sizeof(ip_str)),
		(int)client->nd.addr.sin_port);

	char cmd[CMDLEN];
	int n;
	int state = 1; // Enable
	while((n = read(client->nd.fd, cmd, CMDLEN)) > 0 && state > 0){
		cmd[n] = '\0';
		char* path;
		int cmdid = isValid_cmd(cmd);
		if(cmdid >= 0) {
			switch(cmdid){
				case LS: 
					client_send_dir(&client->nd);
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
					printf("Client quit %s:%d\n",ip_str, (int)client->nd.addr.sin_port);
					state = 0;
					break;
				default:
					break;
			}
		}
	}

	printf("Client disconnected %s:%d\n",ip_str, (int)client->nd.addr.sin_port);
	close(client->nd.fd);
}


