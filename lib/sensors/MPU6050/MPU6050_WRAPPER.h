#ifndef BH1750_WRAPPER_H
#define BH1750_WRAPPER_H

#include "ISensor.h"
#include "MPU6050.h"
#include <map>
#include <string>

class MPU6050Wrapper : public ISensor {
private:
    MPU6050 sensor_;
    bool initialized_ = false;

public:
    MPU6050Wrapper();
    
    bool init() override;
    float read_data(SensorDataTypeIdentifier type) override;
    bool is_initialized() const override;
    SensorType get_type() const override;
    
    bool configure(const std::map<std::string, std::string>& config);
};

#endif