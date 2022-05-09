project(ImplicationBasis)

cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD             17)
set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all -lpthread"
    CACHE STRING "Compiler flags in asan build"
    FORCE
)

find_package(Threads REQUIRED)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "lib/*.cpp")

add_executable(algo.out main.cpp ${SOURCES})

target_link_libraries(algo.out Threads::Threads)
