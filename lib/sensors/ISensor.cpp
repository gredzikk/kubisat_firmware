// ISensor.cpp
#include "ISensor.h"
#include "lib/sensors/BH1750/BH1750_WRAPPER.h"
#include "lib/sensors/BME280/BME280_WRAPPER.h"
#include "lib/sensors/HMC5883L/HMC5883L_WRAPPER.h"
#include "lib/utils.h"

/**
 * @file ISensor.cpp
 * @brief Implements the SensorWrapper class for managing different sensor types.
 * @details This file provides the implementation for initializing, configuring,
 *          and reading data from various sensors.
 */

/**
 * @class SensorWrapper
 * @brief Manages different sensor types and provides a unified interface for accessing sensor data.
 */

/**
 * @brief Provides a global instance of SensorWrapper.
 * @return A reference to the single SensorWrapper instance.
 */
SensorWrapper& SensorWrapper::get_instance() {
    static SensorWrapper instance;
    return instance;
}


/**
 * @brief Default constructor for SensorWrapper.
 */
SensorWrapper::SensorWrapper() = default;


/**
 * @brief Initializes a given sensor type on the specified I2C bus.
 * @param type The sensor type (LIGHT, ENVIRONMENT, etc.).
 * @param i2c The I2C interface pointer.
 * @return True if initialization succeeded, otherwise false.
 */
bool SensorWrapper::sensor_init(SensorType type, i2c_inst_t* i2c) {
    switch(type) {
        case SensorType::LIGHT:
            sensors[type] = new BH1750Wrapper();
            break;
        case SensorType::ENVIRONMENT:
            sensors[type] = new BME280Wrapper(i2c);
            break;
        case SensorType::IMU:
            //sensors[type] = new MPU6050Wrapper(i2c);
            break;
        case SensorType::MAGNETOMETER:
            sensors[type] = new HMC5883LWrapper(i2c);
            break;
    }
    return sensors[type]->init();
}


/**
 * @brief Configures an already initialized sensor with supplied settings.
 * @param type The sensor type.
 * @param config Key-value pairs for sensor configuration.
 * @return True if the sensor was successfully configured, otherwise false.
 */
bool SensorWrapper::sensor_configure(SensorType type, const std::map<std::string, std::string>& config) {
    auto it = sensors.find(type);
    if (it != sensors.end() && it->second->is_initialized()) {
        return it->second->configure(config);
    }
    std::cerr << "Sensor not initialized or not found: " << static_cast<int>(type) << std::endl;
    return false;
}


/**
 * @brief Reads a specific data type (e.g., temperature, humidity) from a sensor.
 * @param sensorType The sensor type.
 * @param dataType The type of data to read (light level, temperature, etc.).
 * @return The requested measurement. Returns 0.0f if sensor not found or uninitialized.
 */
float SensorWrapper::sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType) {
    auto it = sensors.find(sensorType);
    if (it != sensors.end() && it->second->is_initialized()) {
        return it->second->read_data(dataType);
    }
    return 0.0f;
}


/**
 * @brief Retrieves a list of available sensor types with their addresses
 * @return A vector of pairs containing sensor type and I2C address
 */
std::vector<std::pair<SensorType, uint8_t>> SensorWrapper::get_available_sensors() {
    std::vector<std::pair<SensorType, uint8_t>> available_sensors;
    
    for (const auto& sensor_pair : sensors) {
        if (sensor_pair.second->is_initialized()) {
            available_sensors.push_back({sensor_pair.first, sensor_pair.second->get_address()});
        }
    }
    
    return available_sensors;
}


/**
 * @brief Scans the I2C bus for connected sensors
 * @param i2c The I2C interface to scan
 * @return Vector of pairs containing sensor type and I2C address of detected sensors
 */
std::vector<std::pair<SensorType, uint8_t>> SensorWrapper::scan_connected_sensors(i2c_inst_t* i2c) {
    std::vector<std::pair<SensorType, uint8_t>> connected_sensors;
    
    // Define the address ranges to check for each sensor type
    struct SensorAddressInfo {
        SensorType type;
        std::vector<uint8_t> addresses;
    };
    
    std::vector<SensorAddressInfo> sensor_addresses = {
        {SensorType::LIGHT, {0x23, 0x5C}},              // BH1750 addresses
        {SensorType::ENVIRONMENT, {0x76, 0x77}},        // BME280 addresses
        {SensorType::MAGNETOMETER, {0x0D, 0x1E}},       // HMC5883L addresses
        {SensorType::IMU, {0x68, 0x69}}                 // MPU6050 addresses
    };
    
    // Buffer for receiving ACK/NACK
    uint8_t rxdata;
    
    for (const auto& sensor_info : sensor_addresses) {
        for (uint8_t addr : sensor_info.addresses) {
            // Try to read a byte from the device to see if it ACKs
            int result = i2c_read_blocking(i2c, addr, &rxdata, 1, false);
            if (result >= 0) {
                // We received an ACK, so the device exists
                connected_sensors.push_back({sensor_info.type, addr});
                uart_print("Found sensor at address 0x" + std::to_string(addr), VerbosityLevel::DEBUG);
            }
        }
    }
    
    return connected_sensors;
}

