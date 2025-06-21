add_executable(generator generator.cpp)

target_link_libraries(generator PRIVATE
    core
)

if (WIN32)
    target_link_libraries(generator PRIVATE ws2_32)
endif()