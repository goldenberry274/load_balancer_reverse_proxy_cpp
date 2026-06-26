cmake_minimum_required(VERSION 3.16)

project(reverse_proxy_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(load_balancer ${SOURCES})

target_include_directories(load_balancer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(load_balancer PRIVATE yaml-cpp)

if(UNIX)
    target_link_libraries(load_balancer PRIVATE pthread)
endif()

target_compile_options(load_balancer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)