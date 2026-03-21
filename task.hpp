//
// Created by xin on 2026/3/21.
//

#ifndef ZENITH_TASK_HPP
#define ZENITH_TASK_HPP

#include <coroutine>
#include <exception>
namespace zenith {
    template<typename T>
    struct Task {
        /*---------------------------------------------------*/
        //提前声明 内部定义promise_type，此名称必须是固定的
        struct promise_type;
        //coroutine_handle 是一个指向协程内部状态的“指针/句柄”
        //promise_type告诉这个句柄具体长什么样。
        using handle_type = std::coroutine_handle<promise_type>;
        /*----------------------------------------------------*/


        //成员变量 task持有的句柄
        handle_type handle;

        explicit  Task(handle_type h) noexcept :handle(h){}
        ~Task() {if (handle)handle.destroy(); }

        //重载运算符co_await() c++20
        auto operator co_await(){

            struct task_awaiter {
                // task_awaiter 结构体的成员变量
                handle_type callee_handle;

                bool await_ready(){return callee_handle.done();}

                //挂起时执行的逻辑 caller是调用者
                void await_suspend(std::coroutine_handle<> caller) noexcept {
                    callee_handle.promise().waiter = caller;
                    caller.resume();
                }

                T await_resume(){ return callee_handle.promise().value; }
            };
            //返回
            return task_awaiter{handle};
        }

        // void resume() {
        //     if (handle && !handle.done()) {
        //         handle.resume();
        //     }
        // }
        // T get_result() {
        //     return handle.promise().value;
        // }



        //内部定义promise_type，此名称必须是固定的
        struct promise_type {
            //返回co_return 的结果
            T value;

            //<>里面不写是代码通用类型，
            //保存”是谁在co_await“,方便唤醒
            std::coroutine_handle<> waiter;

            //协程创建后的行为:立刻挂起
            //suspend_always是标准库的永远挂起指令。
            static auto initial_suspend() noexcept { return std::suspend_always{}; }

            struct final_suspend;

            //协程结束后的行为
            static auto final_suspend() noexcept { return std::suspend_always{}; }
            //当co_return 运行时的行为
            void return_value(T v) { value = v; }
            //异常
            static void unhandled_exception() { std::terminate(); }

            //from_promise通过此函数计算出句柄地址
            Task get_return_object() noexcept {
                return Task{handle_type::from_promise(*this)};
            }



            struct final_suspend {
                //返回false代表我要挂起
                bool await_ready() noexcept { return false; }

                //在这里执行唤醒逻辑
                //h代表当前协程的句柄
                void await_suspend(handle_type h) noexcept {
                    if (h.promise().waiter) {
                        h.promise().waiter.resume();
                    }
                }

                void await_resume() noexcept { }
            };
        };
    };
}


#endif //ZENITH_TASK_HPP