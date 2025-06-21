add_executable(c4ai main.cpp)

target_link_libraries(c4ai PRIVATE core vendor)
if (WIN32)
    target_link_libraries(c4ai PRIVATE ws2_32)
endif()

target_include_directories(c4ai PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)