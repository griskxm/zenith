//
// Created by xin on 2026/3/21.
//

#ifndef ZENITH_TASK_HPP
#define ZENITH_TASK_HPP

#include "executor.hpp"

#include <coroutine>
#include <exception>
#include <utility>

namespace zenith {
    //----------定义基础的 Promise 逻辑 (复用代码) began
    struct PromiseBase{
        std::coroutine_handle<> waiter_handle;

        std::suspend_always initial_suspend()noexcept {
            struct InitialAwaiter {
                bool await_ready()const noexcept {return false;}
                void await_suspend(std::coroutine_handle<> h)noexcept {
                    // 将自己注册到调度器中
                    Executor::get_instance().publish(h);
                }
                void await_resume()noexcept {}
            };
            return {};
        }

        struct FinalAwaiter {
            bool await_ready()const noexcept {return false;}
            void await_suspend(std::coroutine_handle<> h)noexcept {
                auto& promise{*(reinterpret_cast<PromiseBase*>(h.address()))};
                if (promise.waiter_handle) {
                    promise.waiter_handle.resume();
                }
            }
            void await_resume()noexcept {}
        };
        FinalAwaiter final_suspend()noexcept {return{};}
        void unhandled_exception()noexcept {std::terminate();}
    };
    //---------------定义基础的 Promise 逻辑 (复用代码) end

    //----------泛型版本 began
    template <typename T>
    class Task {
    public:
        struct promise_type:public PromiseBase {
            T value;
            Task get_return_object() {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            void return_value(T v) {value=std::move(v);}
        };

        using HandleType = std::coroutine_handle<promise_type>;

        //---------构造与析构 began
        explicit Task(HandleType h):handle_(h) {}

        ~Task() {
            if (handle_) {
                handle_.destroy();
            }
        }
        //-----------构造与析构 end

        //---------禁止拷贝
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        //---------支持移动
        Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (handle_) {
                    handle_.destroy();
                }
                handle_ = std::exchange(other.handle_, nullptr);
            }
            return *this;
        }

        //---------external interface (public API)
        void resume() {
            if (handle_ && !handle_.done()) {
                handle_.resume();
            }
        }

        T get_result() { return handle_.promise().value; }

        //Awaiter 实现 began
        struct TaskAwaiter {
            HandleType callee_handle;
            bool await_ready()const noexcept {return callee_handle.done();}
            void await_suspend(std::coroutine_handle<> caller_handle)noexcept {
                callee_handle.promise().waiter_handle=caller_handle;
                callee_handle.resume();
            }
            T await_resume() { return callee_handle.promise().value; }
        };
        //Awaiter 实现 end

        auto operator co_await()noexcept {
            return TaskAwaiter{handle_};
        }
    private:
        HandleType handle_{nullptr};
    };
    //----------泛型版本 end

    //----------特化 void 版本 began
    template <>
    class Task<void> {
    public:
        struct promise_type:public PromiseBase {
            Task get_return_object() {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            void return_void() noexcept {}
        };

        using HandleType = std::coroutine_handle<promise_type>;

        //---------构造与析构 began
        explicit Task(HandleType h):handle_(h) {}

        ~Task() {
            if (handle_) {
                handle_.destroy();
            }
        }
        //-----------构造与析构 end

        //---------禁止拷贝
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        //---------支持移动
        Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (handle_) {
                    handle_.destroy();
                }
                handle_ = std::exchange(other.handle_, nullptr);
            }
            return *this;
        }

        //---------external interface (public API)
        void resume() {
            if (handle_ && !handle_.done()) {
                handle_.resume();
            }
        }

        //Awaiter 实现 began
        struct TaskAwaiter {
            HandleType callee_handle;
            bool await_ready()const noexcept {return callee_handle.done();}
            void await_suspend(std::coroutine_handle<> caller_handle)noexcept {
                callee_handle.promise().waiter_handle=caller_handle;
                callee_handle.resume();
            }
            void await_resume() noexcept {}
        };
        //Awaiter 实现 end

        auto operator co_await() const noexcept {
            return TaskAwaiter{handle_};
        }
    private:
        HandleType handle_{nullptr};
    };
    //----------特化 void 版本 end


}

#endif //ZENITH_TASK_HPP
