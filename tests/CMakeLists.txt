set(TEST_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_round_robin.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/config_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_http_parser.cpp"

    "${PROJECT_SOURCE_DIR}/src/balancer/RoundRobinBalancer.cpp"
    "${PROJECT_SOURCE_DIR}/src/config/config.cpp"
    "${PROJECT_SOURCE_DIR}/src/observability/Logger.cpp"
    "${PROJECT_SOURCE_DIR}/src/proxy/HttpParser.cpp"
)

foreach(source_file IN LISTS TEST_SOURCES)
    if(NOT EXISTS "${source_file}")
        message(FATAL_ERROR "Missing test source: ${source_file}")
    endif()

    message(STATUS "Test source found: ${source_file}")
endforeach()

add_executable(load_balancer_tests ${TEST_SOURCES})

target_include_directories(load_balancer_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/external"
)

target_link_libraries(load_balancer_tests PRIVATE
    pthread
    yaml-cpp
)

add_test(
    NAME LoadBalancerTests
    COMMAND load_balancer_tests
)