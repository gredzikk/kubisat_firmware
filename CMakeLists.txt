# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
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
# ====================================================================================
# Main CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

# Basic project setup
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

# SDK initialization
include(pico_sdk_import.cmake)
project(main C CXX ASM)
pico_sdk_init()

# Create common include interface library
add_library(kubisat_includes INTERFACE)
target_include_directories(kubisat_includes INTERFACE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

# Create location library for GPS functionality
add_library(Location_lib STATIC
    lib/location/gps_collector.cpp
    lib/location/NMEA/NMEA_data.cpp
)

target_include_directories(Location_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/location
    ${CMAKE_SOURCE_DIR}/lib/location/NMEA
)

target_link_libraries(Location_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_uart
)

target_include_directories(Utils_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/utils
)

target_link_libraries(Utils_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_uart
)

# Library dependencies - Order matters!
# Base libraries with no interdependencies come first
add_subdirectory(lib/sensors)
add_subdirectory(lib/storage)
add_subdirectory(lib/storage/pico-vfs)
add_subdirectory(lib/eventman)

# Mid-level libraries
add_subdirectory(lib/clock)
add_subdirectory(lib/powerman)

# Top-level libraries
add_subdirectory(lib/comms)

# Final linking of interdependent libraries - Resolve circular dependencies here
# Note: We don't add these in the individual CMakeLists.txt files to avoid circular dependencies
target_link_libraries(eventman_lib PUBLIC Utils_lib)
target_link_libraries(clock_lib PUBLIC eventman_lib storage_lib Location_lib)
target_link_libraries(PowerManager_lib PUBLIC eventman_lib Utils_lib)
target_link_libraries(Comms_lib PUBLIC eventman_lib clock_lib PowerManager_lib)

# Main target
add_executable(main
    main.cpp
)

# Compiler flags
target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wno-error
)

# File system initialization
pico_enable_filesystem(${CMAKE_PROJECT_NAME} FS_INIT lib/storage/storage.cpp)
pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)

# Link libraries to main executable
target_link_libraries(main PRIVATE
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
    pico_multicore
    Sensors_lib
    Comms_lib
    PowerManager_lib
    storage_lib
    clock_lib
    Location_lib
    Utils_lib
    eventman_lib
)

# Build information
file(READ "${CMAKE_SOURCE_DIR}/cmake/CMakeBuildNumberCache.txt" CURRENT_BUILD_NUMBER)
string(STRIP "${CURRENT_BUILD_NUMBER}" CURRENT_BUILD_NUMBER)
pico_set_program_name(main "kubisat_firmware")
pico_set_program_version(main "0.276")
pico_add_extra_outputs(main)

# Uncomment to enable testing
# add_subdirectory(test)
# pico_add_extra_outputs(test_runner)

# Update build number
add_custom_command(
    TARGET main
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DLANGUAGE:STRING="C"
        -DHEADER_DIR:PATH="${CMAKE_SOURCE_DIR}"
        -DCACHE_DIR:PATH="${CMAKE_SOURCE_DIR}/cmake"
        -P "${CMAKE_SOURCE_DIR}/cmake/build_number.cmake"
)

# Copy and rename main.uf2
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
        "${CMAKE_CURRENT_SOURCE_DIR}/release/${PROJECT_NAME}_v${CURRENT_BUILD_NUMBER}.uf2"
    COMMENT "Copying ${PROJECT_NAME}.uf2 to release directory with build number"
    VERBATIM
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Release artifacts created successfully"
    COMMAND ${CMAKE_COMMAND} -E echo "Include graph generated successfully"
    COMMAND ${CMAKE_COMMAND} -E echo "Build process completed successfully!"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/documentation.cmake)