cmake_minimum_required(VERSION 3.26)
project(osi C)

set(CMAKE_C_STANDARD 11)

add_executable(osi child.c
        parent.c)
