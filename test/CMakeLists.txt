cmake_minimum_required(VERSION 3.13)
project(test_runner C CXX)

add_executable(test_runner
    test_runner.cpp
    comms/test_converters.cpp
    comms/test_frame_build.cpp
    comms/test_frame_coding.cpp
    ${CMAKE_SOURCE_DIR}/unity/src/unity.c 
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/unity/src
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/comms 
    ${CMAKE_SOURCE_DIR}/lib/powerman
)

target_link_libraries(test_runner PRIVATE
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
    Comms_lib 
    PowerManager_lib
)

