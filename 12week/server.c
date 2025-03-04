#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFF_SIZE 1024
#define MAXLINE 1024

int main(int argc, char **argv)
{
	int server_socket = 0;
	int client_socket = 0;
	int client_addr_size =0;

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

	memset(&server_addr, 0, sizeof(server_addr));
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

	//select 함수
	fd_set readfds;
	struct timeval timeout;
	int maxfd;

	//fd_set을 초기화 함(0으로 채움)
	FD_ZERO(&readfds);
	FD_SET(server_socket, &readfds);
	maxfd = server_socket; //검사할 비트 테이블 크기 설정
	
	fd_set copyfds;

	while(1)
	{
		copyfds = readfds;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// select 함수로 입출력 이벤트 대기
		int fd_num = select(maxfd + 1, &copyfds, 0, 0, &timeout);

		// 만약 listen socket로 부터 데이터 입력이라면
		// accept 함수를 호출하고, connected socket를 fd_set에 추가한다.
		if(FD_ISSET(server_socket, &readfds))
		{
			int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
			FD_SET(client_fd, &readfds);
			if (client_fd > maxfd) maxfd = client_fd;
		}
	}

	// 입출력 소켓의 이벤트라면 maxfd 만큼 루프를 돌면서
	// 이벤트가 발생한 소켓인지를 확인 후 데이터를 처리한다.
	for (int i = 0; i < maxfd ; i++)
	{
		if(FD_ISSET(i, &copyfds))
		{
			if( i == server_socket){
				client_addr_size = sizeof(client_addr);
				client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
				if(-1 == client_socket)
				{
					printf("클라이언트 연결 수락 실패\n");
					exit(1);
				}
				FD_SET(client_socket, &readfds);
				if(maxfd<client_socket) maxfd = client_socket;

				
				printf("connected client: %d \n", client_socket);
				// while(1)
				// {
				// 	read(client_socket, buff_rcv, BUFF_SIZE);
			
				// 	printf("receive: %s\n", buff_rcv);
				// 	snprintf(buff_snd, BUFF_SIZE, "%zu : %.1019s", strlen(buff_rcv), buff_rcv);
				// 	write(client_socket, buff_snd, strlen(buff_snd)+1);
				// 	if(strcmp(buff_rcv,"bye") == 0){
				// 		printf("servercheck");
				// 		break;
				// 	}
				// }
			}
			else{
				int readn = read(i, buff, MAXLINE);
				if(readn < 1)
				{
					close(i);
					FD_CLR(server_socket, &readfds);
					break;
				}
				write(i, buff, readn);
			}
		}
	}
}
