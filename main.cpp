#include <iostream>
#include "task.hpp"

zenith::Task<int> my_first_coro() {
    std::cout<<"[Coro]协程开始运行....\n";
    co_return 42;
}

int main() {
    std::cout << "开始协程" << std::endl;
    auto task{my_first_coro()};
    std::cout << "恢复协程" << std::endl;
    task.resume();
    std::cout << "协程返回值" << task.get_result()<< std::endl;
    return 0;
}