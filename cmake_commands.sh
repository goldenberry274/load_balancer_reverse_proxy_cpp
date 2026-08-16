#!/bin/bash

case "$1" in
    -cmake_configure)
        echo "Configuring project..."
        cmake -S . -B build
        ;;

    -cmake_build)
        echo "Building project..."
        cmake --build build --parallel
        ;;

    -cmake_rebuild)
        echo "Removing build directory..."
        rm -rf build

        echo "Configuring project..."
        cmake -S . -B build

        echo "Building project..."
        cmake --build build --parallel
        ;;

    -cmake_clean)
        echo "Removing build directory..."
        rm -rf build
        ;;

    # ----------------------------------------
    # Release build
    # ----------------------------------------

    -release)
        echo "Creating Release build..."
        rm -rf build-release

        cmake -S . -B build-release \
            -DCMAKE_BUILD_TYPE=Release

        cmake --build build-release --parallel
        ;;

    # ----------------------------------------
    # AddressSanitizer
    # ----------------------------------------

    -asan_configure)
        echo "Configuring AddressSanitizer build..."

        cmake -S . -B build-asan \
            -DCMAKE_BUILD_TYPE=Debug \
            -DENABLE_ASAN=ON
        ;;

    -asan_build)
        echo "Building AddressSanitizer version..."
        cmake --build build-asan --parallel
        ;;

    -asan_rebuild)
        echo "Removing previous ASan build..."
        rm -rf build-asan

        echo "Configuring AddressSanitizer build..."
        cmake -S . -B build-asan \
            -DCMAKE_BUILD_TYPE=Debug \
            -DENABLE_ASAN=ON

        echo "Building AddressSanitizer version..."
        cmake --build build-asan --parallel
        ;;

    -asan_run)
        echo "Running load balancer with AddressSanitizer..."
        ./build-asan/load_balancer config.yaml
        ;;

    -asan_test)
        echo "Running tests with AddressSanitizer..."
        ctest \
            --test-dir build-asan \
            --output-on-failure
        ;;

    -asan_clean)
        echo "Removing AddressSanitizer build..."
        rm -rf build-asan
        ;;

    -tsan_configure)
        echo "Configuring ThreadSanitizer build..."

        cmake -S . -B build-tsan \
            -DCMAKE_BUILD_TYPE=Debug \
            -DENABLE_TSAN=ON
        ;;

    -tsan_build)
        echo "Building ThreadSanitizer version..."
        cmake --build build-tsan --parallel
        ;;

    -tsan_rebuild)
        echo "Removing previous TSan build..."
        rm -rf build-tsan

        cmake -S . -B build-tsan \
            -DCMAKE_BUILD_TYPE=Debug \
            -DENABLE_TSAN=ON

        cmake --build build-tsan --parallel
        ;;

    -tsan_run)
        echo "Running load balancer with ThreadSanitizer..."
        ./build-tsan/load_balancer config.yaml
        ;;

    -tsan_clean)
        rm -rf build-tsan
        ;;

    # ----------------------------------------
    # Help
    # ----------------------------------------

    *)
        echo "Usage:"
        echo ""
        echo "Normal build:"
        echo "  bash cmake_commands.sh -cmake_configure"
        echo "  bash cmake_commands.sh -cmake_build"
        echo "  bash cmake_commands.sh -cmake_rebuild"
        echo "  bash cmake_commands.sh -cmake_clean"
        echo ""
        echo "Release build:"
        echo "  bash cmake_commands.sh -release"
        echo ""
        echo "AddressSanitizer:"
        echo "  bash cmake_commands.sh -asan_configure"
        echo "  bash cmake_commands.sh -asan_build"
        echo "  bash cmake_commands.sh -asan_rebuild"
        echo "  bash cmake_commands.sh -asan_run"
        echo "  bash cmake_commands.sh -asan_test"
        echo "  bash cmake_commands.sh -asan_clean"
        echo "ThreadSanitizer:"
        echo "  bash cmake_commands.sh -tsan_configure"
        echo "  bash cmake_commands.sh -tsan_build"
        echo "  bash cmake_commands.sh -tsan_rebuild"
        echo "  bash cmake_commands.sh -tsan_run"
        echo "  bash cmake_commands.sh -tsan_test"
        echo "  bash cmake_commands.sh -tsan_clean"
        exit 1
        ;;
esac