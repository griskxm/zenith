#include "task.hpp"
#include "executor.hpp"
#include <iostream>
#include <chrono>

#include "timer.hpp"

using namespace std::chrono_literals;

zenith::Task<void> task_a() {
    std::cout << "task_a() 开始准备睡眠 100ms" << std::endl;
    co_await zenith::sleep(100ms);
    std::cout << "task_a() 醒了" << std::endl;
    co_return;
}
zenith::Task<void> task_b() {
    std::cout << "task_b() 开始准备睡眠 100ms" << std::endl;
    co_await zenith::sleep(50ms);
    std::cout << "task_b() 醒了" << std::endl;
    co_return;
}
int main() {
    auto t1 = task_a();
    auto t2 = task_b();
    std::cout << "启动调用器" << "\n";
    t1.resume();
    t2.resume();
    zenith::Executor::get_instance().run();
    return 0;
}
