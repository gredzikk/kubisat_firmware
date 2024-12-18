cmake_minimum_required(VERSION 3.12)
project(PowerManager_lib)

add_library(PowerManager_lib STATIC
    PowerManager.cpp
    PowerManager.h
)

target_include_directories(PowerManager_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}    # For INA3221.h access
)

target_link_libraries(PowerManager_lib PUBLIC
    INA3221_lib
    pico_stdlib
    hardware_i2c
)