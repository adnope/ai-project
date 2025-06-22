add_library(core STATIC
    opening_book.cpp
    move_sorter.cpp
    transposition_table.cpp
    position.cpp
    solver.cpp
)

target_link_libraries(core PRIVATE vendor)

target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})