#ifndef HMC5883L_WRAPPER_H
#define HMC5883L_WRAPPER_H

#include "lib/ISensor.h"
#include "HMC5883L.h"

class HMC5883LWrapper : public ISensor {
public:
    HMC5883LWrapper(i2c_inst_t* i2c);
    bool init() override;
    float readData(SensorDataTypeIdentifier type) override;
    bool isInitialized() const override;
    SensorType getType() const override;
    bool configure(const std::map<std::string, std::string>& config) override;

private:
    HMC5883L sensor;
    bool initialized;
};

#endif