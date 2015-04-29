#include "netio.h"
#include "filetransfer.h"

#define WAIT_CMD 0
#define WAIT_SERVER 1

extern char output_path[MAXFILENAME];
extern int datachannel_fd;
const char *CLIENT_OUTPUT_PATH_DEFAULT = "./download/";
const char *MAINMENU = "cd [Directory]\nls\nu [filename]\nd [filename]\nq";

void func_cmd_txt(node *server);
void func_cliinfo(node *server);
void func_mkdir();
void func_receive_dir(node *server);

int state = WAIT_CMD;

int main(int argc, char *argv[]){

	signal(SIGCHLD, sig_chld);
	if(argc < 2){
		printf("usage client <IP Address>\n");
		exit(-1);
	}

	// Create Socket
	int connfd = Socket(AF_INET, SOCK_STREAM, 0);

	// Address Transformation
	sin_addr addr;
	inet_pton(AF_INET, argv[1], &addr);

	struct sockaddr_in servaddr;
	Address(&servaddr, AF_INET, addr, htons(SERV_PORT));
    Connect(connfd, (SA*)&servaddr);

    node server = {connfd, servaddr};

    void *args[1];
    strncpy(output_path, CLIENT_OUTPUT_PATH_DEFAULT, sizeof(output_path));
    
   	pid_t  data_proc_id = init_dataconn(((int)(servaddr.sin_port)) + 1, DATACHANNEL_BACKLOG, handler_datachannel, args);
   	printf("Data process: %d\n",data_proc_id);

   	func_mkdir();
    func_cliinfo(&server);
    func_cmd_txt(&server);
    close(server.fd);
    kill(data_proc_id, SIGKILL);

	return 0;
}

void func_mkdir(){
	int flag = mkdir(CLIENT_OUTPUT_PATH_DEFAULT, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(flag == 0) {
		char cwd[MAXFILENAME];
		getcwd(cwd, sizeof(cwd));
		printf("Create Download Directory at: %s\n",cwd);
	}
}

void func_cliinfo(node *server){
	int data_port = (int)server->addr.sin_port + 1;
	printf("Send client info\n");
	int wn = write(server->fd, &data_port, sizeof(data_port));
	if(wn < 0){
		perror("func_cliinfo: write error.");
		close(server->fd);
		exit(-1);
	}
}

void func_cmd_txt(node *server){
	char ip_str[IPV4_ADDRLEN + 1];
	printf("Connected to server %s:%d\n",
		inet_ntop(AF_INET, &server->addr.sin_addr, ip_str, sizeof(ip_str)),
		SERV_PORT);

	struct pollfd pfds[2];
	pfds[0].fd = fileno(stdin);
	pfds[0].events = POLLRDNORM;
	pfds[1].fd = server->fd;
	pfds[1].events = POLLRDNORM;

	int nready = 2;
	int state = 1;
	printf("\nOptions:\n%s\n",MAINMENU);
	while(1){
		poll(pfds, nready, -1);
		
		// Server ready to read
		if(pfds[1].revents & (POLLRDNORM | POLLERR)){
			char buff[MAXLINE];
			int n = read(server->fd, buff, sizeof(buff));
			if(n <= 0){
				printf("Server terminated\n");
				close(server->fd);
				nready--;
				return;
			}
			buff[n] = '\0';
			printf("%s",buff);
		}
		// Stdin ready to read
		else if(pfds[0].revents & POLLRDNORM){
			char cmd[CMDLEN];
			char *path;
			printf("\nOptions:\n%s\n",MAINMENU);

			if(fetch_cmd(cmd, CMDLEN, stdin) == NULL){
				close(fileno(stdin));
				nready--;
			}

			int cmdid = isValid_cmd(cmd); 
			if(cmdid >= 0) {
				int wn = write(server->fd, cmd, strlen(cmd));
				if(wn < 0){
					perror("func_cmd_txt: write error\n");
					continue;
				}

				switch(cmdid){
					case LS: 
						break;
					case U:
						if((path = fetch_addr(cmd)) != NULL)
							send_file(server, SERV_DATA_PORT, path);
						else
							printf("usage: u [filename]\n");
						break;
					case D:
						break;
					case Q:
						printf("Quit\n");
						close(server->fd);
						return;
					default:
						break;
				}
			}
		}


	}
}
