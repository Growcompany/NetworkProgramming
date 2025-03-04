#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

struct inputs {
    int number1;
    char letter;
    int number2;
};

int main(int argc, char **argv)
{
	int client_socket;

	struct sockaddr_in server_addr;

	struct inputs inputs;

	char buff[BUFF_SIZE+5];
	//char chat[BUFF_SIZE+5];

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
		scanf("%s", buff);

		if( strlen(buff) != 3){
			printf("Input Error Ex) 1+3\n");
			break;
		}

		inputs.number1 = buff[0] -'0';
		inputs.letter = buff[1];
		inputs.number2 = buff[2] -'0';

		int ret = write(client_socket, &inputs, sizeof(inputs));

		if(ret < 0){
				printf("write error\n");
				break;
		}

		if((strlen(buff) == 1) && (buff[0] == 'q')){
				printf("클라접속종료 요청\n");
				break;
		}

		memset(buff, 000, sizeof(buff));
		ret = read(client_socket, buff, BUFF_SIZE);
		if(ret < 0){
				printf("read error\n");
				break;
		}
		printf("%s\n",buff);

		memset(buff, 000, sizeof(buff));
		scanf("%s",buff);
		if( buff[0] == '?'){
			ret = write(client_socket, "?", 1);

			if(ret < 0){
					printf("write error\n");
					break;
			}
		}

		memset(buff, 000, sizeof(buff));
		ret = read(client_socket, buff, BUFF_SIZE);
		if(ret < 0){
				printf("read error\n");
				break;
		}
		printf("Answer: %s\n",buff);

	}
	close(client_socket);
	
	return 0;
}
