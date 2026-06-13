#!/bin/bash

case "$1" in
    -cmake_configure)
        echo "Configuring project..."
        cmake -S . -B build
        ;;

    -cmake_build)
        echo "Building project..."
        cmake --build build
        ;;

    -cmake_rebuild)
        echo "Removing build directory..."
        rm -rf build

        echo "Configuring project..."
        cmake -S . -B build

        echo "Building project..."
        cmake --build build
        ;;

    -cmake_clean)
        echo "Removing build directory..."
        rm -rf build
        ;;

    *)
        echo "Usage:"
        echo "  bash cmake_commands.sh -cmake_configure"
        echo "  bash cmake_commands.sh -cmake_build"
        echo "  bash cmake_commands.sh -cmake_rebuild"
        echo "  bash cmake_commands.sh -cmake_clean"
        exit 1
        ;;
esac