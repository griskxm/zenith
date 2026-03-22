//
// Created by xin on 2026/3/23.
//

#ifndef ZENITH_EXECUTOR_HPP
#define ZENITH_EXECUTOR_HPP
#include <coroutine>
#include <queue>
#include <vector>


namespace zenith{
    /**
     * @brief 极简协程调度器
     * 采用单线程fifo（先进先出）队列管理任务
     */
    class Executor {
    public:
        //------调度器是单例或者全局唯一，禁止拷贝
        Executor()=default;
        Executor(const Executor&)=delete;
        Executor& operator=(const Executor&)=delete;

        /**
         * @brief 将一个协程句柄推入就绪队列
         * @param handle 协程句柄, std::coroutine_handle<> 是所有协程句柄的通用基类
         */
        void publish(std::coroutine_handle<> handle) {
            if (handle) {
                ready_queue.push(handle);
            }
        }

        /**
         * @brief 事件循环： 持续运行知道队列没有任务
         */
        void run() {
            while (!ready_queue.empty()) {
                std::coroutine_handle<> handle{ready_queue.front()};
                ready_queue.pop();
                if (!handle.done()) {
                    handle.resume();
                }
                //注意：如果协程在执行过程中 co_await 了别的任务
                //那些任务会被挂起，知道满足条件再次通过publish 回到队列
            }
        }

        /**
         * @brief 获取单例实例（方便在协程内部访问）
         */
        static Executor& get_instance() {
            static Executor instance;
            return instance;
        }
    private:
        std::queue<std::coroutine_handle<>> ready_queue;
    };
}
#endif //ZENITH_EXECUTOR_HPP