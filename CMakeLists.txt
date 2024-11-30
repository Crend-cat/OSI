cmake_minimum_required(VERSION 3.26)
project(OSI C)

set(CMAKE_C_STANDARD 11)

add_executable(OSI
        lab1/src/child.c
        lab1/src/parent.c
        lab2/src/child.c
        lab2/src/parent.c
)
