#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "INA3221/INA3221.h"
#include <map>
#include <string>
#include <hardware/i2c.h>
#include "pico/stdlib.h"
#include "pico/mutex.h"

class PowerManager {
public:
    PowerManager(i2c_inst_t* i2c);
    bool initialize();
    std::string read_device_ids();
    float get_current_charge_solar();
    float get_current_charge_usb();
    float get_current_charge_total();
    float get_current_draw();
    float get_voltage_battery();
    float get_voltage_5v();
    void configure(const std::map<std::string, std::string>& config);
    bool is_charging_solar();
    bool is_charging_usb();
    
    static constexpr float SOLAR_CURRENT_THRESHOLD = 50.0f;  // mA
    static constexpr float USB_CURRENT_THRESHOLD = 50.0f;    // mA
    static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;     // V
    static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f; // V
    static constexpr float FALL_RATE_THRESHOLD = -0.02f;     // V/sample
    static constexpr int FALLING_TREND_REQUIRED = 3;         // samples

private:
    INA3221 ina3221_;
    bool initialized_;
    recursive_mutex_t powerman_mutex_;
    bool charging_solar_active_ = false;
    bool charging_usb_active_ = false;
};

#endif // POWER_MANAGER_H