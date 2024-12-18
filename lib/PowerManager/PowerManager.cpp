#include "PowerManager.h"
#include <iostream>

PowerManager::PowerManager(i2c_inst_t* i2c) 
    : ina3221(INA3221_ADDR40_GND, i2c) {};

bool PowerManager::initialize() {
    ina3221.begin();
    uint16_t manuf_id = ina3221.getManufID();
    uint16_t die_id = ina3221.getDieID();
    std::cout << "INA3221 Manufacturer ID: 0x" << std::hex << manuf_id 
              << ", Die ID: 0x" << die_id << std::endl;
    
    if (manuf_id == 0x5449 && die_id == 0x3220) {
        initialized = true;
        return true;
    }
    std::cerr << "Failed to initialize INA3221." << std::endl;
    return false;
}

float PowerManager::getCurrentEnergy() {
    if (!initialized) return 0.0f;
    return ina3221.getCurrent_mA(INA3221_CH1); // Adjust channel as needed
}

float PowerManager::getBatteryVoltage() {
    if (!initialized) return 0.0f;
    return ina3221.getVoltage(INA3221_CH1); // Adjust channel as needed
}

void PowerManager::configure(const std::map<std::string, std::string>& config) {
    if (!initialized) return;

    if (config.find("operating_mode") != config.end()) {
        if (config.at("operating_mode") == "continuous") {
            ina3221.setModeContinious();
        }
        // Add other modes if necessary
    }

    if (config.find("averaging_mode") != config.end()) {
        int avg_mode = std::stoi(config.at("averaging_mode"));
        switch(avg_mode) {
            case 1:
                ina3221.setAveragingMode(INA3221_REG_CONF_AVG_1);
                break;
            case 4:
                ina3221.setAveragingMode(INA3221_REG_CONF_AVG_4);
                break;
            case 16:
                ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
                break;
            // Add other cases as needed
            default:
                ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
        }
    }

    // Configure other settings as needed
}