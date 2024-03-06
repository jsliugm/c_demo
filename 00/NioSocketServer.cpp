#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    // 创建套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return -1;
    }

    // 设置服务器地址结构
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); // 选择一个合适的端口号

    // 将套接字绑定到指定地址和端口
    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket." << std::endl;
        close(serverSocket);
        return -1;
    }

    // 监听连接请求
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    // 接受连接
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Client connected." << std::endl;

    // 处理客户端请求...

    // 关闭套接字
    close(clientSocket);
    close(serverSocket);

    return 0;
}
