//
// Created by universe on 2024/3/7.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    int client_socket;
} User;

User users[MAX_CLIENTS];
int num_users = 0;

void handle_registration(int client_socket, const char* message);
void handle_search_user(int client_socket, const char* message);
void handle_add_friend(int client_socket, const char* message);
void handle_show_friends(int client_socket);
void handle_send_message(int client_socket, const char* message);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 创建服务器套接字
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定套接字到指定地址和端口
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 使用 poll 来实现多客户端处理
    struct pollfd fds[MAX_CLIENTS + 1];
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    for (int i = 1; i <= MAX_CLIENTS; ++i) {
        fds[i].fd = -1;
    }

    while (1) {
        // 使用 poll 等待事件
        if (poll(fds, MAX_CLIENTS + 1, -1) == -1) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        }

        // 处理服务器套接字的连接事件
        if (fds[0].revents & POLLIN) {
            if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
                perror("Error accepting connection");
                continue;
            }

            // 将新客户端添加到 poll 中
            for (int i = 1; i <= MAX_CLIENTS; ++i) {
                if (fds[i].fd == -1) {
                    fds[i].fd = client_socket;
                    fds[i].events = POLLIN;
                    break;
                }
            }

            printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // 处理客户端的事件
        for (int i = 1; i <= MAX_CLIENTS; ++i) {
            if (fds[i].fd != -1 && fds[i].revents & POLLIN) {
                char buffer[BUFFER_SIZE];
                int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);

                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';

                    // 根据收到的消息进行处理
                    if (strstr(buffer, "REGISTER") != NULL) {
                        handle_registration(fds[i].fd, buffer);
                    } else if (strstr(buffer, "SEARCH_USER") != NULL) {
                        handle_search_user(fds[i].fd, buffer);
                    } else if (strstr(buffer, "ADD_FRIEND") != NULL) {
                        handle_add_friend(fds[i].fd, buffer);
                    } else if (strstr(buffer, "SHOW_FRIENDS") != NULL) {
                        handle_show_friends(fds[i].fd);
                    } else if (strstr(buffer, "SEND_MESSAGE") != NULL) {
                        handle_send_message(fds[i].fd, buffer);
                    } else {
                        // 处理未知的命令
                        send(fds[i].fd, "Unknown command.", strlen("Unknown command."), 0);
                    }
                }

                if (bytes_received <= 0) {
                    // 客户端断开连接
                    printf("Client disconnected.\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
            }
        }
    }

    // 关闭服务器套接字
    close(server_socket);

    return 0;
}

void handle_registration(int client_socket, const char* message) {
    // 解析消息，提取用户名和密码
    char username[50];
    char password[50];
    sscanf(message, "REGISTER %s %s", username, password);

    // 检查是否已经注册
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(username, users[i].username) == 0) {
            send(client_socket, "User already registered.", strlen("User already registered."), 0);
            return;
        }
    }

    // 将用户添加到用户数组
    strncpy(users[num_users].username, username, sizeof(users[num_users].username));
    strncpy(users[num_users].password, password, sizeof(users[num_users].password));
    users[num_users].client_socket = client_socket;

    // 回复注册成功消息
    send(client_socket, "Registration successful.", strlen("Registration successful."), 0);

    // 增加注册用户数量
    num_users++;
}

void handle_search_user(int client_socket, const char* message) {
    // 解析消息，提取要查找的用户名
    char search_username[50];
    sscanf(message, "SEARCH_USER %s", search_username);

    // 查找用户并回复结果
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(search_username, users[i].username) == 0) {
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "User found: %s", users[i].username);
            send(client_socket, response, strlen(response), 0);
            return;
        }
    }

    send(client_socket, "User not found.", strlen("User not found."), 0);
}

void handle_add_friend(int client_socket, const char* message) {
    // 解析消息，提取要添加的好友用户名
    char friend_username[50];
    sscanf(message, "ADD_FRIEND %s", friend_username);

    // 查找要添加的好友
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(friend_username, users[i].username) == 0) {
            // 在这里可以添加一些逻辑，例如检查是否已经是好友等
            // 这里简化为直接将好友加入到用户的好友列表
            users[i].client_socket = client_socket;
            send(client_socket, "Friend added successfully.", strlen("Friend added successfully."), 0);
            return;
        }
    }

    send(client_socket, "Friend not found.", strlen("Friend not found."), 0);
}

void handle_show_friends(int client_socket) {
    // 构建好友列表消息
    char friend_list[BUFFER_SIZE];
    strcpy(friend_list, "Friends: ");

    for (int i = 0; i < num_users; ++i) {
        if (users[i].client_socket == client_socket) {
            strncat(friend_list, users[i].username, sizeof(friend_list) - strlen(friend_list) - 1);
            strncat(friend_list, " ", sizeof(friend_list) - strlen(friend_list) - 1);
        }
    }

    // 发送好友列表消息
    send(client_socket, friend_list, strlen(friend_list), 0);
}

void handle_send_message(int client_socket, const char* message) {
    // 解析消息，提取要发送消息的好友用户名和消息内容
    char friend_username[50];
    char message_content[BUFFER_SIZE];
    sscanf(message, "SEND_MESSAGE %s %[^\n]", friend_username, message_content);

    // 查找好友并发送消息
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(friend_username, users[i].username) == 0) {
            if (users[i].client_socket != -1) {
                char full_message[BUFFER_SIZE];
                snprintf(full_message, sizeof(full_message), "Message from %s: %s", users[client_socket].username, message_content);
                send(users[i].client_socket, full_message, strlen(full_message), 0);
                send(client_socket, "Message sent successfully.", strlen("Message sent successfully."), 0);
            } else {
                send(client_socket, "Friend is not online.", strlen("Friend is not online."), 0);
            }
            return;
        }
    }

    send(client_socket, "Friend not found.", strlen("Friend not found."), 0);
}
