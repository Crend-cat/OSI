cmake_minimum_required(VERSION 3.26)
project(OSI C)

set(CMAKE_C_STANDARD 11)

add_executable(OSI
        lab1/src/child.c
        lab1/src/parent.c
        lab2/src/child.c
        lab2/src/main2.c
        lab3/src/child.c
        lab3/src/parent.c
        lab4/src/allocator.h
        lab4/src/allocator_2n.c
        lab4/src/allocator_list.c
        lab4/src/main.c
)
