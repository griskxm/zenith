//
// Created by xin on 2026/3/21.
//

#ifndef ZENITH_TASK_HPP
#define ZENITH_TASK_HPP

#include <coroutine>
#include <exception>
#include <utility>

namespace zenith {
    /**
     * Task<T>:协程的外壳类
     * 只要函数返回这个类型，他就是一个协程
     */
    template<typename T>
    class Task {
    public:
        struct promise_type;
        using HandleType = std::coroutine_handle<promise_type>;

        //-----构造与析构
        explicit Task(HandleType h):handle_(h){}
        ~Task() {
            if (handle_){handle_.destroy();}
        }

        //------禁止拷贝
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        //-------支持移动
        Task(Task&& other)noexcept:handle_(std::exchange(other.handle_,nullptr)){}
        Task& operator=(Task&& other)noexcept {
            if (this!=&other) {
                if (handle_){handle_.destroy();}
                handle_=std::exchange(other.handle_,nullptr);
            }
            return *this;
        }

        //-------external interface（public API）
        void resume() {
            if (handle_ && !handle_.done()){handle_.resume();}
        }

        T get_result() {return handle_.promise().value;}

        //------Coroutine waiting interface（awaiter interface）
        struct TaskAwaiter {
            HandleType callee_handle;

            [[nodiscard]] bool await_ready()const noexcept {return callee_handle.done();}

            void await_suspend(std::coroutine_handle<promise_type> caller_handle)noexcept {
                callee_handle.promise().waiter_handle=caller_handle;
                callee_handle.resume();
            }
            T await_resume(){
                return callee_handle.promise().value;
            }
        };

        //--------"Overloading the task class co_await()"
        auto operator co_await()noexcept {
            return TaskAwaiter{handle_};
        }

        //-------Core protocol implementation(promise type)
        struct promise_type {
            T value;
            std::coroutine_handle<> waiter_handle;

            Task get_return_object() {
                return Task{HandleType::from_promise(*this)};
            }

            std::suspend_always initial_suspend()noexcept {return{};}

            //-------Custom FinalAwaiter is used to automatically jump back to the caller when the coroutine ends
            struct FinalAwaiter {
                [[nodiscard]] bool await_ready()const noexcept {return false;}
                void await_suspend(HandleType h)noexcept {
                    if (h.promise().waiter_handle) {
                        h.promise().waiter_handle.resume();
                    }
                }
                void await_resume()noexcept {}
            };
            //---------------

            FinalAwaiter final_suspend()noexcept {return {};}
            void return_value(T v) {
                value=std::move(v);
            }
            void unhandled_exception() {std::terminate();}
        };

    private:
    // public:
        HandleType handle_{nullptr};
    };
}


#endif //ZENITH_TASK_HPP