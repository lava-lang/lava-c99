cmake_minimum_required(VERSION 3.25)
project(Lava C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(Lava lava.c)
