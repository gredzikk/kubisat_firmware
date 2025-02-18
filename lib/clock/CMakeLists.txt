cmake_minimum_required(VERSION 3.12)
project(clock_lib)
add_library(clock_lib STATIC
    DS3231.cpp
    DS3231.h
)
target_include_directories(clock_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(clock_lib PUBLIC
    pico_stdlib
    hardware_i2c
)