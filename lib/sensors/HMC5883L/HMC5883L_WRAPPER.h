#ifndef HMC5883L_WRAPPER_H
#define HMC5883L_WRAPPER_H

#include "ISensor.h"
#include "HMC5883L.h"

class HMC5883LWrapper : public ISensor {
public:
    HMC5883LWrapper(i2c_inst_t* i2c);
    bool init() override;
    float read_data(SensorDataTypeIdentifier type) override;
    bool is_initialized() const override;
    SensorType get_type() const override;
    bool configure(const std::map<std::string, std::string>& config) override;

    uint8_t get_address() const override {
        return 0x0D;
    }

private:
    HMC5883L sensor_;
    bool initialized_;
};

#endif