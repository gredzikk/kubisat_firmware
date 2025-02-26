cmake_minimum_required(VERSION 3.12)
project(PowerManager_lib)

add_library(PowerManager_lib STATIC
    PowerManager.cpp
    PowerManager.h
    INA3221/INA3221.cpp 
    INA3221/INA3221.h    
)

target_include_directories(PowerManager_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/INA3221
)

target_link_libraries(PowerManager_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_i2c
    storage_lib
)