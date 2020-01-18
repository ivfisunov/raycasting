cmake_minimum_required(VERSION 3.15)
project(raycasting C)

set(CMAKE_C_STANDARD 99)

add_executable(raycasting src/main.c)
target_link_libraries(raycasting SDL2 m)