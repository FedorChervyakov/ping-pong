cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc)

project(PingPong VERSION 1.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "-DLOG_USE_COLOR")

add_library(logc STATIC
    "${CMAKE_SOURCE_DIR}/log.c/src/log.c"
)

target_include_directories(logc
    PUBLIC
    "${CMAKE_SOURCE_DIR}/log.c/src"
)

add_subdirectory(src)
