cmake_minimum_required(VERSION 2.8)

project(os-find)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(os-find
        dir_walker.cpp
        os-find.cpp)
