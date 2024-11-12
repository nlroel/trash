cmake_minimum_required(VERSION 3.29)
project(dca1000_socket)

set(CMAKE_CXX_STANDARD 23)

add_executable(dca1000_socket udp_listener.cpp)

target_link_libraries(dca1000_socket
        pthread)

