cmake_minimum_required(VERSION 3.16)

project(reverse_proxy_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(load_balancer
    src/main.cpp
    src/balancer/RoundRobinBalancer.cpp
)

target_include_directories(load_balancer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)