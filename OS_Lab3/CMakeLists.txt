cmake_minimum_required(VERSION 3.20)
project(OS_Lab3 C)

set(CMAKE_C_STANDARD 99)

add_executable(shell shell.c)
add_executable(pthreads_client_server pthreads_client_server.c)

