#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
	int client_socket;

	struct sockaddr_in server_addr;

	char buff[BUFF_SIZE+5];

	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(-1 == client_socket)
	{
		printf("socket 생성 실패\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	if(-1 == connect(client_socket,(struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("접속 실패\n");
		exit(1);
	}
	
	while(1)
	{
		printf("숫자를 맞추시오 :");
		scanf("%s", buff);

		int ret = write(client_socket, buff, strlen(buff) + 1); 
		printf("atoi(buff):%d",atoi(buff));
		if(ret < 0){
			printf("write error\n");
			break;
		}

		ret = read(client_socket, buff, strlen(buff) + 1); 
        if (ret < 0) {
            perror("Read error");
            break;
        }

		printf("Server Answer: %s\n",buff);

		memset(buff, 0, sizeof(buff));
	}
	close(client_socket);
	
	return 0;
}
