add_library(vendor INTERFACE)

target_include_directories(vendor INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)