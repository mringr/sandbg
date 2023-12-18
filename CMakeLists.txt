cmake_minimum_required(VERSION 3.20)
project(sandbg)

set(CMAKE_CXX_STANDARD 20)

add_executable(sandbg sandbg.cpp
        include/debugger.hpp
)

add_executable(test_program examples/test_program.cpp)
set_target_properties(test_program PROPERTIES COMPILE_FLAGS "-g -O0")