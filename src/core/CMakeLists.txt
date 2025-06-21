add_library(core STATIC
    OpeningBook.cpp
    Game.cpp
    MoveSorter.cpp
    TranspositionTable.cpp
    Position.cpp
    Solver.cpp
)

target_link_libraries(core PRIVATE vendor)

target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})