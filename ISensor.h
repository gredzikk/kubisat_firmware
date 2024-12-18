// ISensor.h
#ifndef ISENSOR_H
#define ISENSOR_H

#include <cstdint>
#include "hardware/i2c.h"
#include <map>
#include <string>

enum class SensorType {
    LIGHT,          // BH1750
    ENVIRONMENT,    // BME280
    MAGNETOMETER,   // HMC5883L
    IMU             // MPU6050
};

enum class DataType {
    LIGHT_LEVEL,
    TEMPERATURE,
    PRESSURE,
    HUMIDITY,
    MAG_FIELD_X,
    MAG_FIELD_Y,
    MAG_FIELD_Z,
    GYRO_X,
    GYRO_Y,
    GYRO_Z,
    ACCEL_X,
    ACCEL_Y,
    ACCEL_Z
};

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool init() = 0;
    virtual float readData(DataType type) = 0;
    virtual bool isInitialized() const = 0;
    virtual SensorType getType() const = 0;
    virtual bool configure(const std::map<std::string, std::string>& config) = 0;
};

class SensorWrapper {
public:
    static SensorWrapper& getInstance();
    bool initSensor(SensorType type, i2c_inst_t* i2c = nullptr);
    bool configureSensor(SensorType type, const std::map<std::string, std::string>& config);
    float readSensorData(SensorType sensorType, DataType dataType);

private:
    std::map<SensorType, ISensor*> sensors;
    SensorWrapper();
};

#endif // ISENSOR_H