add_library(core STATIC
    OpeningBook.cpp
    Game.cpp
    MoveSorter.cpp
    TranspositionTable.cpp
    Position.cpp
    Solver.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)