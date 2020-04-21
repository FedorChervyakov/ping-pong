cmake_minimum_required(VERSION 3.10)

configure_file(pingpong.h.in pingpong.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server
    logc
)

target_link_libraries(client
    logc
)
