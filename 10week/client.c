#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024
#define MAXLINE 1024

int main(int argc, char **argv)
{
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
