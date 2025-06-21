add_executable(c4ai main.cpp)

target_link_libraries(c4ai PRIVATE
    core
)

target_link_libraries(c4ai PRIVATE
    vendor
)

if (WIN32)
    target_link_libraries(c4ai PRIVATE ws2_32)
endif()