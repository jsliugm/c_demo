cmake_minimum_required(VERSION 3.17)
project(cpp)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(SocketServer 00/SocketServer.c)
add_executable(EpollDemo 00/EpollDemo.c)
add_executable(PollDemo 00/PollDemo.c)
add_executable(PollSocketServer 00/PollSocketServer.c)
add_executable(PollSocketClient 00/PollSocketClient.c)