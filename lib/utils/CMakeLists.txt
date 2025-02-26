add_library(Utils_lib STATIC
    lib/utils/utils.cpp
    lib/utils/pin_config.cpp
)

target_include_directories(Utils_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/utils
)

target_link_libraries(Utils_lib PUBLIC
    kubisat_includes
    pico_stdlib
    hardware_uart
    pico_multicore  
)