//
// Created by xin on 2026/3/23.
//

#ifndef ZENITH_TIMER_HPP
#define ZENITH_TIMER_HPP
#include "executor.hpp"
#include <chrono>
namespace zenith {
    auto sleep(std::chrono::milliseconds duration) {
        struct SleepAwaiter {
            std::chrono::milliseconds dist;
            bool await_ready() const noexcept{return false;}
            void await_suspend(std::coroutine_handle<> h) const noexcept {
                Executor::get_instance().delay_publish(h, dist);
            }
            void await_resume() const noexcept {}
        };
        return SleepAwaiter{duration};
    }
}
#endif //ZENITH_TIMER_HPP