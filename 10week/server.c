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
	int server_socket = 0;
	int client_socket = 0;
	int client_addr_size =0;
	int state = 0;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff[BUFF_SIZE+5];
	char buff_rcv[BUFF_SIZE];
	char buff_snd[BUFF_SIZE];

	memset(&server_addr, 0x00, sizeof(server_addr));
	memset(&client_addr, 0x00, sizeof(client_addr));

	server_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(-1 == server_socket)
	{
		printf("server socket 생성 실패\n");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	state = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if(-1 == state)
	{
		printf("bind() 실행 에러\n");
		exit(1);
	}
	
	while(1)
	{
		client_addr_size = sizeof(client_addr);

		while(1)
		{
			int ret = recvfrom(server_socket, buff_rcv, MAXLINE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
	
			if(ret < 0){
				printf("read error\n");
				break;
			}

		    printf("receive: %s\n", buff_rcv);
	
			if((strlen(buff_rcv) == 1) && (buff_rcv[0] == 'q')){
				printf("클라접속종료 요청\n");
			}

			ret = sendto(server_socket, buff_rcv, strlen(buff_rcv), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

			if(ret < 0){
					printf("write error\n");
					break;
			}

			//snprintf(buff_snd, MAXLINE, "%zu : %.1019s", strlen(buff_rcv), buff_rcv);

			// if(strcmp(buff_rcv,"bye") == 0){
			// printf("servercheck");
            //             break;
            //         }
		    //sprintf(buff_snd,"%d : %s", strlen(buff_rcv), buff_rcv);
		    //snprintf(buff_snd, BUFF_SIZE, "%zu : %.1019s", strlen(buff_rcv), buff_rcv);
		    //ret = write(client_socket, buff_snd, strlen(buff_snd)+1);
		}
		close(client_socket);
	}
}
