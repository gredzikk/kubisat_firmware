// ISensor.h
#ifndef ISENSOR_H
#define ISENSOR_H

#include <cstdint>
#include "hardware/i2c.h"
#include <map>
#include <string>
#include <vector>

enum class SensorType {
    LIGHT,          // BH1750
    ENVIRONMENT,    // BME280
    MAGNETOMETER,   // HMC5883L
    IMU             // MPU6050
};

enum class SensorDataTypeIdentifier {
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
    virtual float read_data(SensorDataTypeIdentifier type) = 0;
    virtual bool is_initialized() const = 0;
    virtual SensorType get_type() const = 0;
    virtual bool configure(const std::map<std::string, std::string>& config) = 0;
    virtual uint8_t get_address() const = 0;
};

class SensorWrapper {
public:
    static SensorWrapper& get_instance();
    bool sensor_init(SensorType type, i2c_inst_t* i2c = nullptr);
    bool sensor_configure(SensorType type, const std::map<std::string, std::string>& config);
    float sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType);
    ISensor* get_sensor(SensorType type);

    std::vector<std::pair<SensorType, uint8_t>> scan_connected_sensors(i2c_inst_t* i2c);
    std::vector<std::pair<SensorType, uint8_t>> get_available_sensors();
    
private:
    std::map<SensorType, ISensor*> sensors;
    SensorWrapper();
};

#endif // ISENSOR_H