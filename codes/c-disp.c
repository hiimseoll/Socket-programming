//client-monitor
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/un.h>
#include<fcntl.h>
 
#define BUFF_SIZE 1024
//#define FILE_SERVER "./sock_addr"

#ifdef C1
#define FILE_SERVER "./sock_addr_C1"
#endif
#ifdef C2
#define FILE_SERVER "./sock_addr_C2"
#endif
#ifdef C3
#define FILE_SERVER "./sock_addr_C3"
#endif
#ifdef C4
#define FILE_SERVER "./sock_addr_C4"
#endif

void init_unix_socket();
void init_inet_socket();

// for unix socket
int unix_server_socket;
int unix_client_socket;
int unix_client_addr_size;
struct sockaddr_un unix_server_addr;
struct sockaddr_un unix_client_addr;

// for inet socket
int inet_server_socket;
int inet_client_socket;
int inet_client_addr_size;
int unix_byte = 0, inet_byte = 0;
struct sockaddr_in inet_server_addr;
struct sockaddr_in inet_client_addr;
char *server_name;
int server_port;

char unix_buff_rcv[BUFF_SIZE];
char inet_buff_rcv[BUFF_SIZE];
char inet_buff_snd[BUFF_SIZE];
//char *ptr = buff_rcv;

int chat_mode = 0;

int main(int argc, char *argv[]){
    if(argc != 3){
    	printf("argc: %d", argc);
    	printf("Error: two argument is needed\n");
    	exit(1);
    }
    server_name = argv[1];
    server_port = atoi(argv[2]);

	init_unix_socket();
 	init_inet_socket();

 	while(1){
	    if(unix_byte = read(unix_client_socket, unix_buff_rcv, BUFF_SIZE) > 0){
	    	printf("[ME] :%s\n", unix_buff_rcv);
	    	strcpy(inet_buff_snd, unix_buff_rcv);
	    	write(inet_client_socket, inet_buff_snd, BUFF_SIZE);

	    	if(!strcmp(unix_buff_rcv, "3")) break;

	    	memset(unix_buff_rcv, 0 , BUFF_SIZE);
	    	memset(inet_buff_snd, 0 , BUFF_SIZE);
	    }
	    if(inet_byte = read(inet_client_socket, inet_buff_rcv, BUFF_SIZE) > 0){
	    	printf("%s\n", inet_buff_rcv);
	    	memset(inet_buff_rcv, 0, BUFF_SIZE);
	    }
	}
	close(unix_client_socket);
	close(unix_server_socket);
	close(inet_client_socket);
	close(inet_server_socket);
	printf("Comm closed\n.");
    return 0;

}    

void init_unix_socket(){
    if(!access(FILE_SERVER, F_OK))
        unlink(FILE_SERVER);

    unix_server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(unix_server_socket == -1){
        printf("Error: server socket error!\n");
        exit(1);
    }
 
    memset(&unix_server_addr, 0, sizeof(unix_server_addr));
    unix_server_addr.sun_family = AF_UNIX;
    strcpy(unix_server_addr.sun_path, FILE_SERVER);
 
    if(bind(unix_server_socket, (struct sockaddr*)&unix_server_addr, sizeof(unix_server_addr)) ==-1){
        printf("Error: bind error!\n");
        exit(1);
    }

	if(listen(unix_server_socket, 5) == -1){
        printf("Error: listen error! \n");
        exit(1);
    }

    unix_client_addr_size = sizeof(unix_client_addr);
    unix_client_socket = accept(unix_server_socket, (struct sockaddr*)&unix_client_addr, &unix_client_addr_size);

    int flags = fcntl(unix_client_socket, F_GETFL, 0);
    fcntl(unix_client_socket, F_SETFL, flags | O_NONBLOCK);
    
    if(unix_client_socket == -1){
        printf("Error: accept failed!\n");
        exit(1);
    }
    printf("unix_init_done!\n");
}

void init_inet_socket(){
	inet_client_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(inet_client_socket == -1){
		printf("Error: socket create fail!\n");
		exit(1);
	}

	memset(&inet_server_addr, 0, sizeof(inet_server_addr));
	inet_server_addr.sin_family = AF_INET;
	inet_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_server_addr.sin_port = htons(server_port);

	if(connect(inet_client_socket, (struct sockaddr*)&inet_server_addr, sizeof(inet_server_addr)) == -1){
		printf("Error: connect fail!\n");
		exit(1);
	}

	int flags = fcntl(inet_client_socket, F_GETFL, 0);
    fcntl(inet_client_socket, F_SETFL, flags | O_NONBLOCK);

    printf("inet_init_done!\n");
}