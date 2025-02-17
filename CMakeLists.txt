# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(main C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY GRAPHVIZ_GENERATE_DEPENDERS ON)
set(GRAPHVIZ_GRAPH_NAME "KubisatFirmware")
set(GRAPHVIZ_GRAPH_HEADER "node [\n  fontsize = \"12\"\n];")
set(GRAPHVIZ_EXTERNAL_LIBS ON)

# Generate the Graphviz files
add_custom_target(dependency_graph
    COMMAND ${CMAKE_COMMAND} --graphviz=dependency_graph.dot .
    COMMAND dot -Tpng dependency_graph.dot -o dependency_graph.png
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(lib/BH1750)
add_subdirectory(lib/BME280)
add_subdirectory(lib/DS3231)
add_subdirectory(lib/INA3221)
add_subdirectory(lib/MPU6050)
add_subdirectory(lib/HMC5883L)
add_subdirectory(lib/LoRa)
add_subdirectory(lib/PowerManager)
add_subdirectory(lib/pico-vfs)

add_executable(
    main 
    main.cpp 
    lib/ISensor.cpp 
    pin_config.cpp    
    event_manager.cpp
    utils.cpp
    logging.cpp
    commands/commands.cpp
    commands/clock_commands.cpp
    commands/diagnostic_commands.cpp
    commands/gps_commands.cpp
    commands/power_commands.cpp
    frame_model/frame.cpp
    frame_model/send.cpp
    frame_model/receive.cpp
    frame_model/communication.cpp
    frame_model/utils_converters.cpp
    lib/GPS/gps_collector.cpp
    lib/GPS/NMEA/NMEA_data.cpp
    storage.cpp
)

target_compile_options(main PRIVATE 
        -Wall 
        -Wextra 
        -Wno-error  # Disable treating warnings as errors
    )

pico_enable_filesystem(${CMAKE_PROJECT_NAME})

pico_set_program_name(main "kubisat_firmware")
pico_set_program_version(main "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)

# Add the standard library to the build
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
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/frame_model
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/GPS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/GPS/NMEA
)
pico_add_extra_outputs(main)

add_custom_command(
	TARGET main
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
		-DLANGUAGE:STRING="C"
		-DHEADER_DIR:PATH="${CMAKE_SOURCE_DIR}"
		-DCACHE_DIR:PATH="${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/build_number.cmake"
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
        "${CMAKE_CURRENT_SOURCE_DIR}/release/${PROJECT_NAME}.uf2"
    COMMENT "Copying ${PROJECT_NAME}.uf2 to release directory"
)