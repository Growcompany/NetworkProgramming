#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

struct FILEREQUEST {
    char file_name[100];
};

struct FILE_INFO {
    long size;
    char created_time[20];
    char message[100];
};

int main(int argc, char **argv)
{
	int client_socket;

	struct sockaddr_in server_addr;
	struct FILEREQUEST file_request;
    struct FILE_INFO file_info;

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
		printf("Find file ('exit' is End): ");
		scanf("%s", buff); // buff로 받기

		strncpy(file_request.file_name, buff, sizeof(file_request.file_name) - 1); // 구조체에 파일이름 저장
        file_request.file_name[sizeof(file_request.file_name) - 1] = '\0';


        if (strcmp(buff, "exit") == 0) {
            printf("Exit\n");
            break;
        }

		int ret = write(client_socket, buff , strlen(buff) + 1); // 파일이름 보내기
        if (ret < 0) {
            printf("write error\n");
            break;
        }

		memset(buff, 0, sizeof(buff));
        ret = read(client_socket, buff, sizeof(buff));
        if (ret < 0) {
            perror("Read error");
            break;
        }

		// 받은 buffer를 구조체로 변환 후 저장
		ret = sscanf(buff, "%ld|%19[^|]|%99[^\n]", &file_info.size, file_info.created_time, file_info.message);

		if (strcmp(file_info.message, "File not Found") == 0) {
            printf("%s\n", file_info.message);
        } else {
            printf("File_size: %ld bytes\n", file_info.size);
            printf("Created_time: %s\n", file_info.created_time);
        }
	}
	close(client_socket);
	
	return 0;
}
