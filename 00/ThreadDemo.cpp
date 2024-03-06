//
// Created by universe on 2024/2/29.
//

#include <iostream>
#include <thread>

// 线程函数1
void task1(int n) {
    for (int i = 0; i < n; ++i) {
        std::cout << "Task 1 iteration " << i << std::endl;
    }
}

// 线程函数2
void task2(int n) {
    for (int i = 0; i < n; ++i) {
        std::cout << "Task 2 iteration " << i << std::endl;
    }
}

int main() {
    const int iterations = 500;

    // 创建两个线程，并分别传入不同的任务函数
    std::thread thread1(task1, iterations);
    std::thread thread2(task2, iterations);

    // 等待两个线程执行完成
    thread1.join();
    thread2.join();

    std::cout << "Both threads have completed their tasks.\n";

    return 0;
}
