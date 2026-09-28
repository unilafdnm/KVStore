cmake_minimum_required(VERSION 3.16)
project(MiniKV)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(minikv
    src/main.cpp
    src/net/Socket.cpp
)

target_include_directories(
    minikv PRIVATE
    src/net
)