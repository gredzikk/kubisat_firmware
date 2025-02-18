cmake_minimum_required(VERSION 3.13)

# Basic project setup
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

# VS Code Pico extension configuration (only needed once)
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# SDK initialization
include(pico_sdk_import.cmake)
project(main C CXX ASM)
pico_sdk_init()

# Library dependencies
add_subdirectory(lib/BH1750)
add_subdirectory(lib/BME280)
add_subdirectory(lib/DS3231)
add_subdirectory(lib/INA3221)
add_subdirectory(lib/MPU6050)
add_subdirectory(lib/HMC5883L)
add_subdirectory(lib/LoRa)
add_subdirectory(lib/PowerManager)
add_subdirectory(lib/pico-vfs)

# Main executable sources
set(MAIN_SOURCES
    main.cpp 
    lib/ISensor.cpp 
    pin_config.cpp    
    event_manager.cpp
    utils.cpp
    storage.cpp
)

set(COMMAND_SOURCES
    commands/commands.cpp
    commands/clock_commands.cpp
    commands/diagnostic_commands.cpp
    commands/gps_commands.cpp
    commands/power_commands.cpp
    commands/event_commands.cpp
)

set(FRAME_MODEL_SOURCES
    frame_model/frame.cpp
    frame_model/send.cpp
    frame_model/receive.cpp
    frame_model/communication.cpp
    frame_model/utils_converters.cpp
)

set(GPS_SOURCES
    lib/GPS/gps_collector.cpp
    lib/GPS/NMEA/NMEA_data.cpp
)

# Main target
add_executable(main 
    ${MAIN_SOURCES}
    ${COMMAND_SOURCES}
    ${FRAME_MODEL_SOURCES}
    ${GPS_SOURCES}
)

# Compiler flags
target_compile_options(main PRIVATE 
    -Wall 
    -Wextra 
    -Wno-error
)

# Include directories
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/frame_model
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/GPS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/GPS/NMEA
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-vfs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-vfs/vendor/littlefs
)

# Dependencies and configuration
pico_enable_filesystem(${CMAKE_PROJECT_NAME} FS_INIT storage.cpp)
pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)

# Link libraries
target_link_libraries(main PRIVATE
    pico_stdlib
    hardware_spi
    hardware_i2c
    BH1750_lib
    BME280_lib
    DS3231_lib
    INA3221_lib
    LoRa_pico_lib
    HMC5883L_lib
    MPU6050_lib   
    PowerManager_lib
    pico_multicore
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
)

# Build information
pico_set_program_name(main "kubisat_firmware")
pico_set_program_version(main "0.1")
pico_add_extra_outputs(main)

# Build number update
add_custom_command(
    TARGET main
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DLANGUAGE:STRING="C"
        -DHEADER_DIR:PATH="${CMAKE_SOURCE_DIR}"
        -DCACHE_DIR:PATH="${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/build_number.cmake"
)

# Release artifacts
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
        "${CMAKE_CURRENT_SOURCE_DIR}/release/${PROJECT_NAME}.uf2"
    COMMENT "Copying ${PROJECT_NAME}.uf2 to release directory"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/documentation.cmake)