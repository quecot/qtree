cmake_minimum_required(VERSION 3.10)
project(ProjectName LANGUAGES C VERSION 0.1.0)

message(STATUS "CMake ${CMAKE_VERSION}")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_CLANG_TIDY "clang-tidy")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(main src/main.c)

target_compile_options(main PRIVATE
    -Werror
    -Wall
    -Wextra
    -Wpedantic
)
