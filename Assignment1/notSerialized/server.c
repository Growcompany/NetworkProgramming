#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/stat.h>

#define BUFF_SIZE 1024

struct FILEREQUEST {
    char file_name[100];
};

struct FILE_INFO {
    long size;
    char created_time[20];
    char message[100];
};

void Find_file(char *file_name, struct FILE_INFO *file_info) {
    struct stat file;
    if (stat(file_name, &file) == 0) {
        file_info->size = file.st_size;
        strftime(file_info->created_time, 20, "%Y_%m_%d-%H:%M:%S", localtime(&file.st_ctime));
        strcpy(file_info->message, "File found");
    } else {
        file_info->size = 0;
        strcpy(file_info->created_time, "");
        strcpy(file_info->message, "File not Found");
    }
}

int main(int argc, char **argv)
{
	int server_socket = 0;
	int client_socket = 0;
	int client_addr_size =0;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct FILEREQUEST file_request;
    struct FILE_INFO file_info;

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
		
		while(1)
		{
		    int ret = read(client_socket, &file_request, sizeof(file_request));
	
			if(ret < 0){
				printf("read error\n");
				break;
			}

			if(ret == 0)
			{
				printf("접속된 클라연결 종료\n");
            	break;
			}
			
			printf("receive file name: %s\n", file_request.file_name);

			Find_file(file_request.file_name, &file_info);

			printf("File : %s\n", file_info.message);

			ret = write(client_socket, &file_info, sizeof(file_info));
	
			if(ret < 0){
				printf("write error\n");
				break;
			}
		}
		close(client_socket);
	}
}
