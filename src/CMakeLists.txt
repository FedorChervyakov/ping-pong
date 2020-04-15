cmake_minimum_required(VERSION 3.10)

# Find the GCC path
execute_process(
    COMMAND bash -c "which gcc | tr -d '\n'"
    OUTPUT_VARIABLE GCC_PATH
)

set(CMAKE_C_COMPILER ${GCC_PATH})

project(PingPong VERSION 1.0 LANGUAGES C)

configure_file(pingpong.h.in pingpong.h)

add_executable(pp-server server.c)
add_executable(pp-client client.c)

target_include_directories(pp-server
                           PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
target_include_directories(pp-client
                           PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
