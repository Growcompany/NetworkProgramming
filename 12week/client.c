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
	//select 함수
	fd_set readfds;
	struct timeval timeout;
	int maxfd;
	listen_fd = socket();
	bind();
	listen();

	//fd_set을 초기화 함(0으로 채움)
	FD_ZERO(&readfds);
	FD_SET(listen_fd, &readfds);
	maxfd = listen_fd; //검사할 비트 테이블 크기 설정

	fd_set copyfds;
	for(;;)
	{
		copyfds = readfds;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// select 함수로 입출력 이벤트 대기
		fd_num = select(maxfd + 1, &copyfds, 0, 0, &timeout);

		// 만약 listen socket로 부터 데이터 입력이라면
		// accept 함수를 호출하고, connected socket를 fd_set에 추가한다.
		if(FD_ISSET(listen_fd, &readfds))
		{
			client_fd = accept(...);
			FD_SET(client_fd, &readfds);
			if (client_fd > maxfd) maxfd = client_fd;
		}
	}

	// 입출력 소켓의 이벤트라면 maxfd 만큼 루프를 돌면서
	// 이벤트가 발생한 소켓인지를 확인 후 데이터를 처리한다.
	for (i = 0; i < maxfd ; i++)
	{
		if(FD_ISSET(i, &copyfds))
		{
			readn = read(i, buf, MAXLINE);
			if(readn < 1)
			{
				close(i);
				FD_CLR(sockfd, &readfds);
				break;
			}
			write(i, buf, readn);
		}
	}

	int client_socket;
	int server_addr_size =0;

	struct sockaddr_in server_addr;

	char buff[BUFF_SIZE+5];
	//char chat[BUFF_SIZE+5];

	client_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(-1 == client_socket)
	{
		printf("socket 생성 실패\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect(client_socket, &server_addr, sizeof(server_addr));
	// sendto(sockfd, buf, strlen(buf), 0, NULL, len);



	while(1)
	{
		server_addr_size = sizeof(server_addr);

		scanf("%s", buff);
		
		int ret = sendto(client_socket, buff, strlen(buff), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

		if(ret < 0){
				printf("write error\n");
				break;
		}

		if((strlen(buff) == 1) && (buff[0] == 'q')){
				printf("클라접속종료 요청\n");
				break;
		}

		memset(buff, 000, sizeof(buff));


		ret = recvfrom(client_socket, buff, MAXLINE, 0, (struct sockaddr*)&server_addr, &server_addr_size);
		
		if(ret < 0){
				printf("read error\n");
				break;
		}
		printf("echo: %s\n",buff);

		memset(buff, 000, sizeof(buff));
	}
	close(client_socket);
	
	return 0;
}
