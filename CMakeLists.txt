cmake_minimum_required(VERSION 3.28)
project(HAL64 C)

set(CMAKE_C_STANDARD 90)
add_compile_options(-O0)
set(LEXER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${PROJECT_SOURCE_DIR}/autogen/lexer.c")

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(FLEX REQUIRED)

flex_target(lexer src/lexer.l "${LEXER_OUT}" DEFINES_FILE ${LEXER_DIR}/lexer.h)

file(GLOB_RECURSE SOURCE "src/*.c")

add_executable(HAL64 main.c ${SOURCE} ${LEXER_OUT})
