#ifndef BH1750_WRAPPER_H
#define BH1750_WRAPPER_H

#include "lib/ISensor.h"
#include "BH1750.h"
#include <map>
#include <string>

class BH1750Wrapper : public ISensor {
private:
    BH1750 sensor;
    bool initialized = false;

public:
    BH1750Wrapper();
    int get_i2c_addr();
    bool init() override;
    float readData(SensorDataTypeIdentifier type) override;
    bool isInitialized() const override;
    SensorType getType() const override;
    
    bool configure(const std::map<std::string, std::string>& config);

};

#endif // BH1750_WRAPPER_H