//
// Created by universe on 2024/3/5.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>

#define MAX_EVENTS 10
#define PORT 8080

int main() {
    int server_socket, client_socket, events;
    struct sockaddr_in server_addr, client_addr;
    socklen_t clientAddrLen = sizeof(client_addr);
    struct pollfd fds[MAX_EVENTS];

    // 初始化 pollfd 结构
    for (int i = 0; i < MAX_EVENTS; i++) {
        fds[i].fd = -1; // 初始时设置为无效的文件描述符
        fds[i].events = POLLIN|POLLOUT;
    }

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
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // 调用 poll 函数等待事件发生
        if (poll(fds, MAX_EVENTS, -1) == -1) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        }

        // 检查监听的文件描述符上是否有事件发生
        for (int i = 0; i < MAX_EVENTS; i++) {
            if (fds[i].revents & POLLIN) {
                // 有新连接
                if (i == 0) {
                    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &clientAddrLen);
                    if (client_socket == -1) {
                        perror("Error accepting connection");
                        exit(EXIT_FAILURE);
                    }
                    printf("New connection from %s\n", inet_ntoa(client_addr.sin_addr));

                    // 将新连接的 socket 添加到 pollfd 结构中
                    for (int j = 1; j < MAX_EVENTS; j++) {
                        if (fds[j].fd == -1) {
                            fds[j].fd = client_socket;
                            fds[j].events = POLLIN|POLLOUT;
                            break;
                        }
                    }
                } else {
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
                        printf("Received data: %s\n", buffer);
                    }
                }
            }
            if (fds[i].revents & POLLOUT) {
                // 可以写数据
                // 在实际应用中，可以在此处进行写操作
                printf("Ready to write data\n");
            }
        }
    }

    // 关闭服务器 socket
    close(server_socket);

    return 0;
}
