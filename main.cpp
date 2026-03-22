#include <iostream>
#include "task.hpp"
#include <thread>
#include <chrono>
using namespace zenith;

// 模拟一个异步函数（最简单版）
Task<int> async_add(int a, int b) {
    std::cout << "async_add 开始\n";

    // 模拟耗时操作
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    std::cout << "async_add 完成\n";
    co_return a + b;
}
// 另一个协程调用前一个协程
Task<int> test_co_await() {
    std::cout << "[test] 将要 co_await async_add\n";

    int result = co_await async_add(10, 25);

    std::cout << "[test] 得到结果：" << result << "\n";
    co_return result * 2;
}
int main() {
    std::cout << "main 开始\n";

    auto t = test_co_await();

    std::cout << "协程已创建，但尚未运行\n";
    t.resume();
    auto msg{t.get_result()};
    std::cout<<"main最终测试结果" <<msg<<"\n";
    std::cout << "测试结束"<<"\n";
    return 0;
}