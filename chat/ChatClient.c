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
#define MAX_EVENTS 2

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_EVENTS];

    // 创建 socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // 初始化 pollfd 结构
    fds[0].fd = client_socket;
    fds[0].events = POLLIN;

    // 第二个 pollfd 用于检查标准输入
    fds[1].fd = 0;  // 0 表示标准输入
    fds[1].events = POLLIN;

    printf("Connected to server on port %d\n", PORT);

    while (1) {
        // 调用 poll 函数等待事件发生
        if (poll(fds, MAX_EVENTS, -1) == -1) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        }

        // 检查服务端 socket 是否有数据可读
        if (fds[0].revents & POLLIN) {
            char buffer[1024];
            ssize_t bytes_read = read(fds[0].fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                // 服务端关闭连接
                printf("Connection closed by server\n");
                break;
            } else {
                buffer[bytes_read] = '\0';
                printf("Received data from server: %s\n", buffer);
            }
        }

        // 检查标准输入是否有数据可读
        if (fds[1].revents & POLLIN) {
            // 从标准输入读取数据
            char input_buffer[1024];
            fgets(input_buffer, sizeof(input_buffer), stdin);

            // 发送输入的数据到服务端
            write(client_socket, input_buffer, strlen(input_buffer));
        }
    }

    // 关闭客户端 socket
    close(client_socket);

    return 0;
}
