//
// Created by universe on 2024/3/6.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>

#define PORT 8080
#define MAX_EVENTS 10
#define MAX_CLIENTS 10

typedef struct {
    int id;
    int fd;
} Client;

int main() {
    int server_socket, events, client_id = 1;
    struct sockaddr_in server_addr, client_addr;
    struct pollfd fds[MAX_EVENTS];
    Client clients[MAX_CLIENTS];

    // 创建监听 socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定服务器地址
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_socket, 5) == -1) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    // 初始化 pollfd 结构
    for (int i = 0; i < MAX_EVENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

    fds[0].fd = server_socket;

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // 调用 poll 函数等待事件发生
        if (poll(fds, MAX_EVENTS, -1) == -1) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        }

        // 检查监听的文件描述符上是否有事件发生
        for (int i = 0; i < MAX_EVENTS; i++) {
            if (fds[i].fd == server_socket && fds[i].revents & POLLIN) {
                // 有新连接
                int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, NULL);
                if (client_socket == -1) {
                    perror("Error accepting connection");
                    exit(EXIT_FAILURE);
                }
                printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // 分配 ID 给新连接的客户端
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == -1) {
                        clients[j].id = client_id++;
                        clients[j].fd = client_socket;
                        printf("Assigned ID %d to client\n", clients[j].id);
                        break;
                    }
                }

                // 将新连接的 socket 添加到 pollfd 结构中
                for (int j = 0; j < MAX_EVENTS; j++) {
                    if (fds[j].fd == -1) {
                        fds[j].fd = client_socket;
                        break;
                    }
                }
            } else if (fds[i].revents & POLLIN) {
                // 有数据可读
                char buffer[1024];
                ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    // 客户端关闭连接
                    close(fds[i].fd);
                    printf("Connection closed\n");

                    // 从 pollfd 结构中移除关闭的连接
                    fds[i].fd = -1;
                } else {
                    // 处理读取到的数据，这里简单打印
                    buffer[bytes_read] = '\0';
                    printf("Received message: %s\n", buffer);

                    // 解析消息中的 ID 和内容
                    int id;
                    char message[1024];
                    if (sscanf(buffer, "%d %s", &id, message) == 2) {
                        // 查找目标客户端并转发消息
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].id == id && clients[j].fd != -1) {
                                write(clients[j].fd, message, strlen(message));
                                printf("Message forwarded to client with ID %d\n", id);
                                break;
                            }
                        }
                    } else if (strcmp(buffer, "q\n") == 0) {
                        // 查询在线客户端 ID
                        printf("Online client IDs: ");
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].fd != -1) {
                                printf("%d ", clients[j].id);
                            }
                        }
                        printf("\n");
                    }
                }
            }
        }
    }

    // 关闭服务器 socket
    close(server_socket);

    return 0;
}

