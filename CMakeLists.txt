cmake_minimum_required(VERSION 3.22)
project(project C)

set(CMAKE_C_STANDARD 99 -Wall -Wextra -Wpedantic -Werror -Wno-cast-function-type)

add_executable(project main.c)
