cmake_minimum_required(VERSION 3.14)
project(c4ai VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra)

add_subdirectory(external)

add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(src/utils)

set(CMAKE_BUILD_TYPE Debug)