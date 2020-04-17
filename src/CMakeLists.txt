cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc)

project(PingPong VERSION 1.0 LANGUAGES C)

configure_file(pingpong.h.in pingpong.h)

add_executable(server server.c)
add_executable(client client.c)

target_include_directories(server
                           PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
target_include_directories(client
                           PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
