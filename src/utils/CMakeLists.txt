add_executable(generator generator.cpp)

target_link_libraries(generator PRIVATE core)

target_include_directories(generator PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)