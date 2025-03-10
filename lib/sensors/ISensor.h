#ifndef ISENSOR_H
#define ISENSOR_H

#include <map>
#include <string>
#include <vector>
#include <utility>
#include "hardware/i2c.h"

enum class SensorType : uint8_t {
    NONE = 0x00,
    LIGHT = 0x01,
    ENVIRONMENT = 0x02,
};

enum class SensorDataTypeIdentifier : uint8_t {
    NONE = 0x00,
    LIGHT_LEVEL = 0x01,
    TEMPERATURE = 0x02,
    HUMIDITY = 0x03,
    PRESSURE = 0x04,
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
    static SensorWrapper& get_instance() {
        static SensorWrapper instance;
        return instance;
    }

    bool sensor_init(SensorType type, i2c_inst_t* i2c = nullptr);
    bool sensor_configure(SensorType type, const std::map<std::string, std::string>& config);
    float sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType);
    ISensor* get_sensor(SensorType type);

    std::vector<std::pair<SensorType, uint8_t>> scan_connected_sensors(i2c_inst_t* i2c);
    std::vector<std::pair<SensorType, uint8_t>> get_available_sensors();

private:
    std::map<SensorType, ISensor*> sensors;
    SensorWrapper() = default;
};

#endif