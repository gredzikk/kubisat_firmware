// ISensor.cpp
#include "ISensor.h"
#include "lib/BH1750/BH1750_WRAPPER.h"
#include "lib/BME280/BME280_WRAPPER.h"
#include "lib/HMC5883L/HMC5883L_WRAPPER.h"

/**
 * @brief Provides a global instance of SensorWrapper.
 * @return A reference to the single SensorWrapper instance.
 */
SensorWrapper& SensorWrapper::getInstance() {
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
bool SensorWrapper::initSensor(SensorType type, i2c_inst_t* i2c) {
    switch(type) {
        case SensorType::LIGHT:
            sensors[type] = new BH1750Wrapper();
            break;
        case SensorType::ENVIRONMENT:
            sensors[type] = new BME280Wrapper(i2c);
            break;
        // case SensorType::IMU:
        //     sensors[type] = new MPU6050Wrapper(i2c);
        //     break;
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
bool SensorWrapper::configureSensor(SensorType type, const std::map<std::string, std::string>& config) {
    auto it = sensors.find(type);
    if (it != sensors.end() && it->second->isInitialized()) {
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
float SensorWrapper::readSensorData(SensorType sensorType, SensorDataTypeIdentifier dataType) {
    auto it = sensors.find(sensorType);
    if (it != sensors.end() && it->second->isInitialized()) {
        return it->second->readData(dataType);
    }
    return 0.0f;
}