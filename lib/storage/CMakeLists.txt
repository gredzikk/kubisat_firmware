cmake_minimum_required(VERSION 3.12)
project(storage_lib)

add_library(storage_lib STATIC
    storage.cpp
    storage.h
)

target_include_directories(storage_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(storage_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_flash
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
)