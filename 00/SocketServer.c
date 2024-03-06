//
// Created by universe on 2024/3/5.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // 创建套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // 将套接字绑定到指定地址和端口
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(serverSocket, MAX_CONNECTIONS) == -1) {
        perror("Error listening on socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 接受连接请求
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Error accepting connection");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // 从客户端接收数据
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        // 处理接收到的数据，这里简单地将数据回送给客户端
        send(clientSocket, buffer, bytesRead, 0);
    }

    if (bytesRead == 0) {
        // 客户端关闭了连接
        printf("Client disconnected.\n");
    } else if (bytesRead == -1) {
        // 接收数据时出现错误
        perror("Error receiving data");
    }

    // 关闭套接字
    close(clientSocket);
    close(serverSocket);

    return 0;
}
