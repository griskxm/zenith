
#include "task.hpp"
#include "executor.hpp"
#include <iostream>
using namespace zenith;

Task<int> sub_task() {
    std::cout << "sub_task()" << std::endl;
    co_return 0;
}
Task<void> main_task() {
    std::cout << "main_task()" << std::endl;
    int v=co_await sub_task();
    std::cout << "main_task result" << std::endl;
    co_return;
}
int main() {
    auto t=main_task();
    std::cout << "启动调度器" << std::endl;
    Executor::get_instance().run();
    return 0;
}