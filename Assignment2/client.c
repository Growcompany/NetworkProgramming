#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define BUFF_SIZE 1024
#define MAXLINE 1024

volatile int running = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFF_SIZE];

    while (1) {
		memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화
        int ret = read(client_socket, buffer, sizeof(buffer) - 1);
        if (ret <= 0) {
            printf("read error from server.\n");
            close(client_socket);

			pthread_mutex_lock(&mutex);
            running = 0;
            pthread_mutex_unlock(&mutex);

            break;
        }

		buffer[ret] = '\0';
		printf("server -> %s", buffer);

		if (strncmp(buffer, "bye", 3) == 0) {
			printf("server에서 bye 요청\n");
			close(client_socket);

			pthread_mutex_lock(&mutex);
            running = 0;
            pthread_mutex_unlock(&mutex);

			break;
		}
    }

	return 0;
}

int main(int argc, char **argv)
{
	int client_socket;
	int server_addr_size =0;

	struct sockaddr_in server_addr;

	char buff[BUFF_SIZE+5];

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == client_socket)
	{
		printf("socket 생성 실패\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		perror("Connect failed");
		close(client_socket);
		exit(EXIT_FAILURE);
	}

	// 서버로부터 메시지 받기 멀티쓰레드로 실행
	pthread_t receive_thread;
	pthread_create(&receive_thread, NULL, receive_messages, &client_socket);

	while (1) {
		pthread_mutex_lock(&mutex);
        if (!running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

		// 입력 대기
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);

		struct timeval timeout = {0, 50000}; // 50ms 타임아웃

		int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout); // scanf와 메시지 받기를 동시에 하기 위해 select이용

		if (ret > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
			scanf("%s", buff);
			write(client_socket, buff, strlen(buff));

			if (strncmp(buff, "bye", 3) == 0) {
				pthread_mutex_lock(&mutex);
				running = 0;
				pthread_mutex_unlock(&mutex);
				break;
			}
		}

	}
	pthread_join(receive_thread, NULL);
    close(client_socket);
    return 0;
}
