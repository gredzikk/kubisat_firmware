cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
project(test_runner C CXX)


add_executable(test_runner
${CMAKE_SOURCE_DIR}/unity/src/unity.c 
    test_runner.cpp
    test_clock.cpp
    test_powerman.cpp
    test_frame.cpp
    ${CMAKE_SOURCE_DIR}/lib/telemetry/telemetry_manager.cpp 
    ${CMAKE_SOURCE_DIR}/lib/utils.cpp
)

# Include all directories needed
target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/unity/src
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/comms
    ${CMAKE_SOURCE_DIR}/lib/powerman
    ${CMAKE_SOURCE_DIR}/lib/sensors
    ${CMAKE_SOURCE_DIR}/lib/storage
    ${CMAKE_SOURCE_DIR}/lib/location
    ${CMAKE_SOURCE_DIR}/lib/clock
    ${CMAKE_SOURCE_DIR}/lib/telemetry
)

# Link all libraries used by the main application
target_link_libraries(test_runner PRIVATE
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
    pico_multicore
    Comms_lib
    PowerManager_lib
    eventman_lib        
    Sensors_lib
    storage_lib         
    clock_lib
    telemetry_lib
    commands_lib
)

pico_add_extra_outputs(test_runner)