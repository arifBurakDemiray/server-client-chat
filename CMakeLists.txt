
cmake_minimum_required(VERSION 3.16)

project(ceng421_02T C)

set(CMAKE_BUILD_TYPE Release)

add_executable(client client.c time_lib.c)

add_executable(server server.c time_lib.c)

 