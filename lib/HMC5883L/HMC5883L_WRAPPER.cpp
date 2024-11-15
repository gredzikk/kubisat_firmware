#include "HMC5883L_WRAPPER.h"
#include <cmath>
#include <cstdio>

HMC5883LWrapper::HMC5883LWrapper(i2c_inst_t* i2c) : sensor(i2c), initialized(false) {}

bool HMC5883LWrapper::init() {
    initialized = sensor.init();
    return initialized;
}

float HMC5883LWrapper::readData(DataType type) {
    if (!initialized) return 0.0f;

    int16_t x, y, z;
    if (!sensor.read(x, y, z)) return 0.0f;

    const float LSB_TO_UT = 100.0 / 1090.0;
    float x_uT = x * LSB_TO_UT;
    float y_uT = y * LSB_TO_UT;
    float z_uT = z * LSB_TO_UT;

    switch (type) {
        case DataType::MAG_FIELD_X:
            return x_uT;
        case DataType::MAG_FIEDL_Y:
            return y_uT;
        case DataType::MAG_FIELD_Z:
            return z_uT;
        default:
            return 0.0f;
    }
}

bool HMC5883LWrapper::isInitialized() const {
    return initialized;
}

SensorType HMC5883LWrapper::getType() const {
    return SensorType::MAGNETOMETER;
}

bool HMC5883LWrapper::configure(const std::map<std::string, std::string>& config) {
    // Configuration logic if needed
    return true;
}