cmake_minimum_required(VERSION 3.14)
project(ai_project LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
add_compile_options(-Wall -Wextra)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/core
    ${PROJECT_SOURCE_DIR}/include/external
)

# Get all .cpp files in src/
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Loop over each .cpp file to create an executable
foreach(src_file ${SRC_FILES})
    # Get the filename without extension
    get_filename_component(exec_name ${src_file} NAME_WE)

    # Create executable
    add_executable(${exec_name} ${src_file})

    # (Optional) Set target properties here if needed
    # target_include_directories(${exec_name} PRIVATE ...)
    # target_compile_options(${exec_name} PRIVATE ...)
endforeach()

# Set default build type
set(CMAKE_BUILD_TYPE Debug)