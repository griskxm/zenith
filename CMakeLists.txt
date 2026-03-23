cmake_minimum_required(VERSION 4.2)
project(zenith)

set(CMAKE_CXX_STANDARD 20)

add_executable(zenith main.cpp
        task.hpp
        executor.hpp
        timer.hpp)
