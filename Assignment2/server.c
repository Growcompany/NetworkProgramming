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
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    char name[BUFF_SIZE];
} Client;

Client *clients[MAX_CLIENTS] = {NULL};
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

#define BUFFER_SIZE 1024       // 총 버퍼 크기

void write_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    char formatted_message[BUFFER_SIZE];
    snprintf(formatted_message, sizeof(formatted_message), "%s\n", message); // 개행 문자 추가

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket != sender_socket) {
            write(clients[i]->socket, formatted_message, strlen(formatted_message));
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_pthread(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    char name[BUFFER_SIZE];

	int client_id = -1;

    // 클라이언트 자리 찾기
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = malloc(sizeof(Client));
			if (clients[i] == NULL) {
                perror("Failed to allocate memory for client");
                pthread_mutex_unlock(&clients_mutex);
                close(client_socket);
                return NULL;
            }
            clients[i]->socket = client_socket;
            snprintf(clients[i]->name, sizeof(clients[i]->name), "client%d", i + 1);
            client_id = i;
			//printf("Assigned %s to client socket %d\n", clients[i]->name, client_socket);
			printf("New Assigned %s\n", clients[i]->name);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (client_id == -1) {
        // 클라이언트 초과
        write(client_socket, "Server full.\n", 13);
        close(client_socket);
        return NULL;
    }

	// 새로 접속된 클라이언트에게만 할당된 클라이름 전송
	char message[BUFF_SIZE];
	snprintf(message, sizeof(message)+1, "%s\n", clients[client_id]->name); // 이름 + 개행 문자
	write(client_socket, message, strlen(message)); // 클라이언트로 전송



    while (1) {
        int ret = read(client_socket, buffer, sizeof(buffer) - 1);
        if (ret <= 0) {
            break;
        }
        buffer[ret] = '\0';

        if (strcmp(buffer, "bye") == 0) {
            snprintf(buffer, sizeof(buffer), "%s", clients[client_id]->name);
			printf("%s bye\n", buffer);
            write_message(buffer, client_socket);
            break;
        } else {
            write_message(buffer, client_socket);
        }
    }

    // 클라이언트 종료전에 자리 비우기
    pthread_mutex_lock(&clients_mutex);
    free(clients[client_id]);
    clients[client_id] = NULL;
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    return NULL;
}

void *server_input(void *arg) {
    char buffer[BUFFER_SIZE];
    char target_client[BUFFER_SIZE];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // 개행 문자 제거

        if (strncmp(buffer, "all ", 4) == 0) { // "all "로 시작하는 메시지 처리
            if (strcmp(buffer + 4, "bye") == 0) { // "all bye"일 경우
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_CLIENTS; i++) { // 모든 클라 다 종료
                    if (clients[i] != NULL) {
                        write(clients[i]->socket, "bye\n", 4); // 클라에 bye 보내기
                        close(clients[i]->socket);
                        free(clients[i]);
                        clients[i] = NULL;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);
                printf("All clients disconnected.\n");
            } else {
                write_message(buffer + 4, -1); // "all [input]" 경우
            }
        }  else if (strcmp(buffer, "allbye") == 0) { 
            // "allbye" 명령 처리
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != NULL) {
                    write(clients[i]->socket, "bye\n", 4);
                    close(clients[i]->socket);
                    free(clients[i]);
                    clients[i] = NULL;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            printf("All clients disconnected.\n");
		} else if (sscanf(buffer, "%s -> %[^\n]", target_client, buffer) == 2) { // ex) client1 -> hi 를 잘라서 입력받기
            // 특정 클라이언트 메시지 처리
            pthread_mutex_lock(&clients_mutex);
            int found = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != NULL && strcmp(clients[i]->name, target_client) == 0) {
                    found = 1;
                    if (strcmp(buffer, "bye") == 0) {
                        // 클라이언트 종료 처리
                        write(clients[i]->socket, "bye\n", 4);
                        close(clients[i]->socket);
                        free(clients[i]);
                        clients[i] = NULL;
                        printf("client %s disconnected.\n", target_client);
                    } else {
						// 버퍼 끝에 개행 문자 추가
						size_t buffer_len = strlen(buffer);
						if (buffer_len < sizeof(buffer) - 2) { // 여유 공간이 있는지 확인
							buffer[buffer_len] = '\n';
							buffer[buffer_len + 1] = '\0'; // NULL 종결
						}
						write(clients[i]->socket, buffer, strlen(buffer)); // 실제 문자열 길이만 전송
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            if (!found) {
                printf("client %s not found.\n", target_client);
            }
        } else if (strcmp(buffer, "q") == 0) {
            // "q" 로 서버 종료
            printf("Exit server.\n");
            exit(0);
        } else {
            printf("Input Error. ex) client1 -> hi, all hi, allbye, all bye,\n");
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
	int server_socket = 0;
	int client_socket = 0;
	int client_addr_size =0;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	memset(&server_addr, 0x00, sizeof(server_addr));
	memset(&client_addr, 0x00, sizeof(client_addr));

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
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

	// 서버 입력 처리 스레드
    pthread_t input_thread;
    pthread_create(&input_thread, NULL, server_input, NULL);
    pthread_detach(input_thread);

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
		if(-1 == client_socket)
		{
			printf("클라이언트 연결 수락 실패\n");
			exit(1);
		}

		int *new_sock = malloc(sizeof(int)); //클라소켓 새로 만들기
		*new_sock = client_socket;

		// 클라소켓 멀티 쓰레드로 처리
		pthread_t client_thread;
		pthread_create(&client_thread, NULL, client_pthread, new_sock);
		pthread_detach(client_thread);
	}

	close(server_socket);
	return 0;
}
