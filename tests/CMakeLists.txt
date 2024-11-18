add_library(unity STATIC
    ${CMAKE_SOURCE_DIR}/lib/Unity/src/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/Unity/src
)

# Example test executable
add_executable(sensor_tests
    test_sensors.cpp
)

target_link_libraries(sensor_tests
    unity
    BH1750_lib
    BME280_lib
    INA3221_lib
    HMC5883L_lib
    pico_stdlib
)