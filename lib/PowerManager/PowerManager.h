// [lib/PowerManager/PowerManager.h](lib/PowerManager/PowerManager.h)
#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "INA3221.h"
#include <map>
#include <string>
#include <hardware/i2c.h>
#include "pico/stdlib.h"

class PowerManager {
public:
    PowerManager(i2c_inst_t* i2c);
    bool initialize();
    std::string readIDs();
    float getCurrentChargeSolar();
    float getCurrentChargeUSB();
    float getCurrentChargeTotal();
    float getCurrentDraw();
    float getVoltageBattery();
    float getVoltage5V();
    void configure(const std::map<std::string, std::string>& config);

private:
    INA3221 ina3221;
    bool initialized;
};

#endif // POWER_MANAGER_H