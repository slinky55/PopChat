cmake_minimum_required(VERSION 3.26)
project(PopChat C)

set(CMAKE_C_STANDARD 11)

add_executable(PopChat main.c
        log.h
        log.c)
