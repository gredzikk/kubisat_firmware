cmake_minimum_required(VERSION 3.12)

# Add subdirectories for each sensor
add_subdirectory(BH1750)
add_subdirectory(BME280)
add_subdirectory(MPU6050)
add_subdirectory(HMC5883L)

# Create a sensor library that includes all sensor libraries
add_library(Sensors_lib INTERFACE)

# No need for include directories since the libraries already expose them
target_link_libraries(Sensors_lib INTERFACE
    kubisat_includes
    BH1750_lib
    BME280_lib
    MPU6050_lib
    HMC5883L_lib
)