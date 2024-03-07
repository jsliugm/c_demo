//
// Created by universe on 2024/3/7.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void send_request(int client_socket, const char* request);

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // 创建客户端套接字
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    // 示例：用户注册
    send_request(client_socket, "REGISTER User1 Pass123");

    // 示例：查找用户
    send_request(client_socket, "SEARCH_USER User2");

    // 示例：添加好友
    send_request(client_socket, "ADD_FRIEND User2");

    // 示例：显示好友列表
    send_request(client_socket, "SHOW_FRIENDS");

    // 示例：发送消息
    send_request(client_socket, "SEND_MESSAGE User2 Hello, User2!");

    // 关闭客户端套接字
    close(client_socket);

    return 0;
}

void send_request(int client_socket, const char* request) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;

    // 发送请求
    bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent == -1) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    // 接收响应
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Error receiving response");
        exit(EXIT_FAILURE);
    }

    // 打印响应
    buffer[bytes_received] = '\0';
    printf("Response from server: %s\n", buffer);
}
