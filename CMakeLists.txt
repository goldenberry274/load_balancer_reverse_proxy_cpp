cmake_minimum_required(VERSION 3.16)

project(reverse_proxy_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(load_balancer ${SOURCES})

target_include_directories(load_balancer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
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

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot be enabled together")
endif()

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "AddressSanitizer enabled")

        target_compile_options(load_balancer PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
            -g
        )

        target_link_options(load_balancer PRIVATE
            -fsanitize=address
        )
    endif()
endif()

if(ENABLE_TSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "ThreadSanitizer enabled")

        target_compile_options(load_balancer PRIVATE
            -fsanitize=thread
            -fno-omit-frame-pointer
            -g
        )

        target_link_options(load_balancer PRIVATE
            -fsanitize=thread
        )
    endif()
endif()

enable_testing()
add_subdirectory(tests)