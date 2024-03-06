//
// Created by universe on 2024/3/5.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define PORT 8080

int main() {
    int server_socket, client_socket, epoll_fd, event_count;
    struct sockaddr_in server_addr, client_addr;
    struct epoll_event events[MAX_EVENTS];

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

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error creating epoll instance");
        exit(EXIT_FAILURE);
    }

    // 将服务器 socket 注册到 epoll 中
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
        perror("Error adding server socket to epoll");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // 等待事件发生
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            perror("Error in epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == server_socket) {
                // 有新连接
                client_socket = accept(server_socket, (struct sockaddr*)&client_addr, NULL);
                if (client_socket == -1) {
                    perror("Error accepting connection");
                    exit(EXIT_FAILURE);
                }
                printf("New connection from %s\n", inet_ntoa(client_addr.sin_addr));

                // 将新连接的 socket 注册到 epoll 中
                ev.events = EPOLLIN;
                ev.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
                    perror("Error adding client socket to epoll");
                    exit(EXIT_FAILURE);
                }
            } else {
                // 有数据可读
                char buffer[1024];
                ssize_t bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    // 客户端关闭连接
                    close(events[i].data.fd);
                    printf("Connection closed\n");
                } else {
                    // 处理读取到的数据，这里简单打印
                    buffer[bytes_read] = '\0';
                    printf("Received data: %s\n", buffer);
                }
            }
        }
    }

    // 关闭服务器 socket
    close(server_socket);

    return 0;
}
