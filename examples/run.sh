#!/bin/bash

set -e

PrintHelp() {
    echo "Usage: $0 [OPTION] <EXAMPLE-NAME>"
    echo "Compile and run the example"
    echo ""
    echo "With no OPTION, compile and run the release build"
    echo ""
    echo "-h, --help           Print help"
    echo "-d, --debug          Compile the debug build and run it with gdb"
    echo "-w, --windows        Compile the Windows build and run it with Wine"
    echo "-p, --profile        Compile the profile build, profile it with perf and display the data with hotspot"
    echo "-m, --memory-leak    Compile the memory leak build and run it"
    echo ""
    echo "Available EXAMPLE-NAMEs"
    for f in src/*.cpp; do echo "$(basename ${f%.*})"; done
}

# Help info
if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    PrintHelp

# Debug build
elif [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    if [ -z "$2" ]; then
        PrintHelp
        exit 1
    fi
    clear
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build build_debug -j$(nproc)
    gdb -ex run --args ./build_debug/bin/$2

# Windows build
elif [ "$1" == "-w" ] || [ "$1" == "--windows" ]; then
    if [ -z "$2" ]; then
        PrintHelp
        exit 1
    fi
    clear
    cmake -B build_windows -DCMAKE_TOOLCHAIN_FILE="$(pwd)/mingw-w64-x86_64.cmake" -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build_windows -j$(nproc)
    wine ./build_windows/bin/$2.exe

# Profile build
elif [ "$1" == "-p" ] || [ "$1" == "--profile" ]; then
    if [ -z "$2" ]; then
        PrintHelp
        exit 1
    fi
    clear
    cmake -B build_profile -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer"
    cmake --build build_profile -j$(nproc)
    perf record -g --call-graph dwarf ./build_profile/bin/$2
    hotspot perf.data

# Memory leak build
elif [ "$1" == "-m" ] || [ "$1" == "--memory-leak" ]; then
    if [ -z "$2" ]; then
        PrintHelp
        exit 1
    fi
    clear
    cmake -B build_memory_leak -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=address"
    cmake --build build_memory_leak -j$(nproc)
    ./build_memory_leak/bin/$2

# Release build
else
    if [ -z "$1" ]; then
        PrintHelp
        exit 1
    fi
    clear
    cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build -j$(nproc)
    cp build/compile_commands.json ../build
    ./build/bin/$1
fi
