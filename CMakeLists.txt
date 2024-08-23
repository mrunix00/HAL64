cmake_minimum_required(VERSION 3.28)
project(HAL64 C)

set(CMAKE_C_STANDARD 90)

set(LEXER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${LEXER_DIR}/lexer.c")

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(FLEX REQUIRED)

flex_target(lexer autogen/lexer.l "${LEXER_OUT}" DEFINES_FILE ${LEXER_DIR}/lexer.h)

file(GLOB_RECURSE  "src/*.c")

add_executable(HAL64 main.c src/utils/memory.c ${LEXER_OUT})
