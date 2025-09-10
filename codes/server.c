//server.c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/un.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<stdbool.h>
#include<sys/ipc.h>
#include<pthread.h>
#include<unistd.h>

 
#define BUFF_SIZE 1024
#define MAX_ROOM_CAPACITY 5
#define MAX_CAPACITY 15

void init_socket();
void send_list(int);
void accept_client();
void sigint_handler();
void waiting_room();
void enter_chat(int);
void init_shared();
void check_msg();

	
pthread_t tid1, tid2;

//공유메모리
int shmid1, shmid2, shmid3;
int (*shared_cli_per_room)[6];
char *shared_msg;


int client_socket_for_parent[15];
int server_socket;
int client_socket;
int client_addr_size;
int n = 0;
int client_cnt = 0;
char buff_snd[BUFF_SIZE];
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
char buff_rcv[BUFF_SIZE];
char *ptr = buff_rcv;
char *menu_list[5] = {
	"<MENU>",
	"1. 채팅방 목록 보기",
	"2. 채팅방 참여하기(사용법: 2 <채팅방 번호>)",
	"3. 프로그램 종료",
	"(0을 입력하면 메뉴가 다시 표시됩니다.)"
};
char *room_list[5] = {
	"<ChatRoom info>",
	"[ID: 0] ROOM-0 ",
	"[ID: 1] ROOM-1 ",
	"[ID: 2] ROOM-2 ",""
};

 
int main(void){

	init_shared();
	signal(SIGINT, sigint_handler);
	init_socket();

	pthread_create(&tid1, NULL, (void *(*)(void *)) accept_client, NULL);
	pthread_create(&tid2, NULL, (void *(*)(void *)) check_msg, NULL);

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
}

void waiting_room(){
	send_list(0);
	printf("[MAIN] 새로운 사용자가 접속했습니다: %d\n", client_cnt);

	while((n = recv(client_socket, ptr, sizeof(buff_rcv), 0))){

		printf("[MAIN] 사용자 %d 메시지: %s\n", client_cnt, buff_rcv);

		if(!strcmp(buff_rcv, "0")){
			send_list(0);
		}
		else if(!strcmp(buff_rcv, "1")){
			send_list(1);
		}
		else if(buff_rcv[0] == '2'){
			enter_chat(buff_rcv[2] - '0');
		}
		else if(!strcmp(buff_rcv, "3")){
			printf("[MAIN] %d 사용자와의 접속을 해제합니다.\n", client_cnt);
			break;
		}
        memset(buff_rcv, 0, BUFF_SIZE);
	}
    close(client_socket);
}

void enter_chat(int room_num){
	if(shared_cli_per_room[room_num][0] >= MAX_ROOM_CAPACITY) return;
	else{
		++shared_cli_per_room[room_num][0];
		shared_cli_per_room[room_num][client_cnt] = 1;
	}

	printf("[MAIN] 사용자 %d가 채팅방 %d에 참여합니다.\n", client_cnt, room_num);
	

	while((n = recv(client_socket, ptr, sizeof(buff_rcv), 0))){
		printf("[Ch.%d] 사용자 %d의 메시지: %s\n", room_num, client_cnt, buff_rcv);
		if(!strcmp(buff_rcv, "quit")) break;

		if(shared_cli_per_room[room_num][0] == 1) printf("[Ch.%d] 사용자 %d가 혼자여서 메시지를 전달안합니다.\n", room_num, client_cnt);
		else{
			sprintf(shared_msg, "[%d] %s", client_cnt, buff_rcv);
		}
		memset(buff_rcv, 0 , BUFF_SIZE);
	}
	printf("[Ch.%d] 사용자 %d를 채팅방에서 제거합니다.\n", room_num, client_cnt);
	--shared_cli_per_room[room_num][0];
	shared_cli_per_room[room_num][client_cnt] = 0;
}

void init_socket(){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket == -1){
        printf("Error: server socket error!\n");
        exit(1);
    }
 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8081);

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==-1 ){
        printf("Error: bind error!\n");
        exit(1);
    }

    if(listen(server_socket, 16) == -1){
        printf("Error: listen error! \n");
        exit(1);
    }

    client_addr_size = sizeof(client_addr);

    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

    printf("init done!\n\n");
}

void accept_client(){
	while(true){
		if((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size)) == -1){
			exit(1);
		}
		else{
			client_socket_for_parent[client_cnt] = client_socket;
			client_cnt += 1;	
		} 
		switch(fork()){
		case -1:
			printf("Error: fork failed!\n");
			break;
		case 0:
			close(server_socket);
			waiting_room();
			break;
		default:
			break;
		}
	}
	if(n == 0){
		printf("연결 종료");
	}
	close(client_socket);
}

void send_list(int which){
	char **ptr = ((which == 0) ? menu_list : room_list);
	for(int i = 0; i < 5; i++){
		strcpy(buff_snd, ptr[i]);
		if(which == 1 && (i > 0 && i < 4)){
			char temp[20];
			sprintf(temp, "(%d/%d)", shared_cli_per_room[i - 1][0], MAX_ROOM_CAPACITY);
			strcat(buff_snd, temp);
		}
		write(client_socket, buff_snd, strlen(buff_snd)+1);
		memset(buff_snd, 0, BUFF_SIZE);
		usleep(1000);
	}
}

void sigint_handler(){

	printf("(시그널 핸들러) 마무리 작업 시작!\n");
	pthread_kill(tid1, 0);
	pthread_kill(tid2, 0);
	shmdt(shared_cli_per_room);
	shmdt(shared_msg);
	shmctl(shmid1, IPC_RMID, NULL);
	shmctl(shmid2, IPC_RMID, NULL);
	close(server_socket);
	close(client_socket);
	exit(EXIT_SUCCESS);
}

void init_shared(){
	shmid1 = shmget(IPC_PRIVATE, 3 * 6 * sizeof(int), IPC_CREAT | 0666);
	shmid2 = shmget(IPC_PRIVATE, BUFF_SIZE, IPC_CREAT | 0666);
	if(shmid1 < 0 || shmid2 < 0){
		perror("shmget");
		exit(1);
	}

	shared_cli_per_room = shmat(shmid1, NULL, 0);
	shared_msg = (char *)shmat(shmid2, NULL, 0);
	if (shared_cli_per_room == (void *) -1 || shared_msg == (void *) -1) {
   	 	perror("shmat");
    	exit(1);
	}


	for(int i = 0; i < 3; i++){
		for(int n = 0; n < 6; n++){
			shared_cli_per_room[i][n] = 0;
		}
	}
	memset(shared_msg, 0, BUFF_SIZE);
}

void check_msg(){
	while(true){
		usleep(1000);
		if(shared_msg[0] != '[') continue;

		int room;
		for(int i = 0; i < 3; i++){
			if(shared_cli_per_room[i][shared_msg[1] - '0'] == 1){
				room = i;
			}
		}
		for(int i = 1; i <= MAX_ROOM_CAPACITY; i++){
			if(shared_msg[1] - '0' == i) continue;
			if(shared_cli_per_room[room][i] == 1){
				write(client_socket_for_parent[i - 1], shared_msg, strlen(shared_msg) + 1);
			}
		}
		memset(shared_msg, 0, BUFF_SIZE);
	}
}