#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ipc.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
	int server_socket = 0;
	int client_socket = 0;
	int client_addr_size =0;
	// int *shared_array;
	// int shmid;
	pid_t pid = 0;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff[BUFF_SIZE+5];
	char buff_rcv[BUFF_SIZE];
	char buff_snd[BUFF_SIZE];

	memset(&server_addr, 0x00, sizeof(server_addr));
	memset(&client_addr, 0x00, sizeof(client_addr));

	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(-1 == server_socket)
	{
		printf("server socket 생성 실패\n");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(-1 == bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("bind() 실행 에러\n");
		exit(1);
	}

	if(-1 == listen(server_socket, 5))
	{
		printf("listen() 실행 실패\n");
		exit(1);
	}
	
	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
		
		if(-1 == client_socket)
		{
			printf("클라이언트 연결 수락 실패\n");
			continue;
		}

		// // 공유 메모리 생성
		// shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | 0666);
		// if (shmid == -1) {
		// 	perror("shmget");
		// 	exit(1);
		// }

		// // 공유 메모리 연결
		// shared_array = (int *)shmat(shmid, NULL, 0);
		// if (shared_array == (int *)-1) {
		// 	perror("shmat");
		// 	exit(1);
		// }

		int correct_number = 10; 

		pid = fork();
		if(pid == 0)
		{
			while((read(client_socket, buff, strlen(buff) + 1)) > 0){
				
				int receive_number = atoi(buff);
				
				if( receive_number < correct_number ){
					strcpy(buff, "Up!!"); 
				}
				else if( receive_number > correct_number ){
					strcpy(buff, "Low!!"); 
				}
				else if( receive_number == correct_number ){
					strcpy(buff, "Correct!!"); 
				}

				int ret = write(client_socket, buff, strlen(buff) + 1);
				if(ret < 0){
					printf("write error\n");
					break;
				}
				memset(buff, 0, sizeof(buff));
			}
			close(client_socket);
		}
	}
}
