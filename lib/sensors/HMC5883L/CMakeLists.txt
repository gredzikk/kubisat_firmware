cmake_minimum_required(VERSION 3.12)
project(HMC5883L_lib)

add_library(HMC5883L_lib STATIC
    HMC5883L.cpp
    HMC5883L.h
    HMC5883L_WRAPPER.cpp
    HMC5883L_WRAPPER.h
)

target_include_directories(HMC5883L_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/sensors
)

target_link_libraries(HMC5883L_lib PUBLIC
    pico_stdlib
    hardware_i2c
)