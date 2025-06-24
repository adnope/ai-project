add_executable(c4ai main.cpp game/game.cpp)

target_link_libraries(c4ai PRIVATE vendor core)
if (WIN32)
    target_link_libraries(c4ai PRIVATE ws2_32)
endif()

target_include_directories(c4ai PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory(game)
add_subdirectory(gui)