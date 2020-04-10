cmake_minimum_required(VERSION 3.10)

project(PingPong)

# Find the GCC path
execute_process(
    COMMAND bash -c "which gcc | tr -d '\n'"
    OUTPUT_VARIABLE GCC_PATH
)

set(CMAKE_C_COMPILER ${GCC_PATH})

add_executable(pp-server server.c)
add_executable(pp-client client.c)
