#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    // 创建套接字
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return -1;
    }

    // 设置服务器地址结构
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器的IP地址
    serverAddr.sin_port = htons(8080); // 服务器的端口号

    // 连接到服务器
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        close(clientSocket);
        return -1;
    }

    std::cout << "Connected to server." << std::endl;

    // 发送和接收数据...

    // 关闭套接字
    close(clientSocket);

    return 0;
}
