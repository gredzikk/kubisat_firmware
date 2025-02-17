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
    bool isSolarActive() const;
    bool isUSBConnected() const;
    
    static constexpr float SOLAR_CURRENT_THRESHOLD = 50.0f;  // mA
    static constexpr float USB_CURRENT_THRESHOLD = 50.0f;    // mA
    static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;     // V
    static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f; // V
    static constexpr float FALL_RATE_THRESHOLD = -0.02f;     // V/sample
    static constexpr int FALLING_TREND_REQUIRED = 3;         // samples

private:
    INA3221 ina3221;
    bool initialized;

    bool solarActive = false;
    bool usbConnected = false;
};

#endif // POWER_MANAGER_H