//client-input.c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/un.h>  
 
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
 
int main(){
    int client_socket;
    struct sockaddr_un server_addr;
    char buff_snd[BUFF_SIZE];
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(client_socket == -1){
        printf("Error: socket create fail!\n");
        exit(1);
    }
 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, FILE_SERVER);
 
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("Error: connect fail!\n");
        exit(1);
    }

    while(1){
    	printf("Enter: ");
    	fgets(buff_snd, sizeof(buff_snd), stdin);
    	buff_snd[strlen(buff_snd) - 1] = '\0';
    	write(client_socket, buff_snd, strlen(buff_snd) + 1);

    	if(!strcmp(buff_snd, "3")) break;

    	memset(buff_snd, 0, BUFF_SIZE);
    	usleep(1000);
    }	
    close(client_socket);
    printf("Comm closed\n.");
    return 0;
}
