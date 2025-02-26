cmake_minimum_required(VERSION 3.12)
project(eventman_lib)

add_library(eventman_lib STATIC
    event_manager.cpp
)

target_include_directories(eventman_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eventman_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_sync
    hardware_flash
)