# C++ 协程综述与本项目 Task 实现说明

本文档分为两部分：
1. C++ 协程的整体概念与关键组件。
2. 本项目 `task.hpp` 中使用到的标准库类型/函数说明。

## 1. C++ 协程核心概念

C++20 协程把一个函数变成“可挂起、可恢复执行”的状态机。只要函数返回一个“协程返回对象”，编译器就会把它变成协程并生成对应的状态机。

关键组件如下：

### 1.1 协程返回对象（coroutine return object）
- 协程函数的返回类型，例如本项目里的 `Task<T>`。
- 该类型必须定义一个嵌套的 `promise_type`，并通过 `promise_type::get_return_object()` 创建并返回协程对象。

### 1.2 promise_type
- 协程的核心协议类型，承载协程状态和返回值。
- 由编译器在协程创建时构造。
- 负责：
  - 返回协程对象（`get_return_object`）。
  - 决定初始与结束时的挂起行为（`initial_suspend`、`final_suspend`/自定义 awaiter）。
  - 接收 `co_return` 的值（`return_value`）。
  - 处理异常（`unhandled_exception`）。

### 1.3 协程句柄（coroutine handle）
- 类型：`std::coroutine_handle<promise_type>`。
- 类似轻量级指针，指向协程帧，提供：
  - `resume()`：恢复执行协程。
  - `done()`：判断协程是否结束。
  - `destroy()`：销毁协程帧并释放资源。
- 通过 `std::coroutine_handle<promise_type>::from_promise(*promise)` 获取。

### 1.4 suspend points 与 awaiter
- 关键字 `co_await` 会将表达式转换成“awaiter”。
- awaiter 必须提供三个方法：
  - `await_ready()`：若返回 true 则不挂起。
  - `await_suspend(coroutine_handle)`：挂起时执行，通常用于安排何时恢复。
  - `await_resume()`：恢复后返回 `co_await` 表达式的结果。
- 协程启动和结束也各有一个“隐式 awaiter”控制挂起行为：
  - `initial_suspend()` 位置。
  - `final_suspend()` 或等价的自定义 awaiter。

### 1.5 对称/非对称协程切换
- 一个协程在 `await_suspend()` 中可以恢复另一个协程，形成“对称式”控制转移。
- 本项目中 `TaskAwaiter::await_suspend` 会先保存 caller 的句柄，然后立即恢复 callee，使执行权转移到被等待的协程。
- 协程结束时通过 `FinalAwaiter` 再把控制权还给等待者。

## 2. `task.hpp` 中使用的标准库类型与函数

`task.hpp` 使用了 `<coroutine>`、`<exception>`、`<utility>` 头文件。下面按实际用法说明。

### 2.1 来自 `<coroutine>` 的类型

1. `std::coroutine_handle<promise_type>`
- 用途：保存并操作协程帧。
- 在 `Task<T>` 中通过 `HandleType` 别名使用。
- 关键成员函数：
  - `resume()`：恢复协程执行。
  - `done()`：判断协程是否结束。
  - `destroy()`：销毁协程帧。
  - `from_promise(promise_type&)`：从 `promise_type` 获取协程句柄。

2. `std::coroutine_handle<>`
- 类型擦除版本的协程句柄，不绑定具体 promise 类型。
- 本项目中用来保存 `waiter_handle`（等待者协程）。

3. `std::suspend_always`
- awaiter 类型，`await_ready()` 恒为 false，表示一定会挂起。
- `promise_type::initial_suspend()` 返回 `std::suspend_always`，因此协程在创建后默认先挂起，直到显式 `resume()`。

### 2.2 来自 `<utility>` 的函数

1. `std::exchange`
- 用途：在移动操作中安全“转移并置空”协程句柄。
- 示例：`handle_ = std::exchange(other.handle_, nullptr)`。
- 在 `Task` 的移动构造与移动赋值中使用。

2. `std::move`
- 用途：在 `return_value` 中把返回值移动进 `promise_type::value`。
- 形式：`value = std::move(v)`。

### 2.3 来自 `<exception>` 的函数

1. `std::terminate`
- 用途：在 `promise_type::unhandled_exception()` 中处理未捕获异常。
- 当前实现直接终止程序，这是最简单的处理方式。

## 3. `Task<T>` 的运行流程（结合代码）

1. 协程创建：
- 编译器生成 `promise_type` 并调用 `get_return_object()` 返回 `Task<T>`。
- `initial_suspend()` 返回 `std::suspend_always`，协程先挂起。

2. 外部启动：
- 调用 `Task::resume()`，如果未结束就恢复协程执行。

3. 协程等待：
- `co_await Task<T>` 时触发 `Task::operator co_await()`。
- `TaskAwaiter::await_suspend()` 保存 caller 协程句柄，然后立即恢复 callee 协程。

4. 协程结束：
- `co_return` 触发 `promise_type::return_value()` 保存结果。
- `FinalAwaiter::await_suspend()` 将控制权返回给等待者。

5. 资源释放：
- `Task` 析构时调用 `handle_.destroy()` 释放协程帧。

## 4. 注意点与可改进项（仅说明）

- `promise_type` 中的 `waier_handle` 看起来像拼写错误，实际意图应为 `waiter_handle`。
- 当前实现未处理异常传播，仅调用 `std::terminate()`，需要更完整的异常模型时可扩展。
- `Task::get_result()` 直接读取 `promise().value`，未检查协程是否已完成。

