#include "ISensor.h"
#include "lib/sensors/BH1750/BH1750_WRAPPER.h"
#include "lib/sensors/BME280/BME280_WRAPPER.h"
#include "lib/utils.h"

bool SensorWrapper::sensor_init(SensorType type, i2c_inst_t* i2c) {
    switch (type) {
    case SensorType::LIGHT:
        sensors[type] = new BH1750Wrapper(i2c);
        break;
    case SensorType::ENVIRONMENT:
        sensors[type] = new BME280Wrapper(i2c);
        break;
    default:
        return false;
    }
    return sensors[type]->init();
}

bool SensorWrapper::sensor_configure(SensorType type, const std::map<std::string, std::string>& config) {
    if (sensors.find(type) == sensors.end()) {
        return false;
    }
    return sensors[type]->configure(config);
}

float SensorWrapper::sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType) {
    if (sensors.find(sensorType) == sensors.end()) {
        return -1.0f;
    }
    return sensors[sensorType]->read_data(dataType);
}

ISensor* SensorWrapper::get_sensor(SensorType type) {
    return sensors[type];
}

std::vector<std::pair<SensorType, uint8_t>> SensorWrapper::scan_connected_sensors(i2c_inst_t* i2c) {
    std::vector<std::pair<SensorType, uint8_t>> connectedSensors;

    // Scan for BH1750 (Light Sensor)
    BH1750Wrapper lightSensor(i2c);
    if (lightSensor.init()) {
        connectedSensors.push_back(std::make_pair(SensorType::LIGHT, lightSensor.get_address()));
    }

    // Scan for BME280 (Environment Sensor)
    BME280Wrapper environmentSensor(i2c);
    if (environmentSensor.init()) {
        connectedSensors.push_back(std::make_pair(SensorType::ENVIRONMENT, environmentSensor.get_address()));
    }

    return connectedSensors;
}

std::vector<std::pair<SensorType, uint8_t>> SensorWrapper::get_available_sensors() {
    std::vector<std::pair<SensorType, uint8_t>> availableSensors;
    for (const auto& sensorPair : sensors) {
        availableSensors.push_back(std::make_pair(sensorPair.first, sensorPair.second->get_address()));
    }
    return availableSensors;
}