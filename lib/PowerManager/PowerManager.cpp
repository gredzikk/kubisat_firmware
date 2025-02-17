#include "PowerManager.h"
#include <iostream>

PowerManager::PowerManager(i2c_inst_t* i2c) 
    : ina3221(INA3221_ADDR40_GND, i2c) {};

bool PowerManager::initialize() {
    initialized = ina3221.begin();
    return initialized;
}

std::string PowerManager::readIDs() {
    if (!initialized) return "noinit";
    std::string MAN = "MAN " + std::to_string(ina3221.getManufID());
    std::string DIE = "DIE " + std::to_string(ina3221.getDieID());
    return MAN + " - " + DIE;
}

float PowerManager::getVoltageBattery() {
    if (!initialized) return 0.0f;
    return ina3221.getVoltage(INA3221_CH1); 
}

float PowerManager::getVoltage5V() {
    if (!initialized) return 0.0f;
    return ina3221.getVoltage(INA3221_CH2); 
}

float PowerManager::getCurrentChargeUSB() {
    if (!initialized) return 0.0f;
    return ina3221.getCurrent_mA(INA3221_CH1); 
}

float PowerManager::getCurrentDraw() {
    if (!initialized) return 0.0f;
    return ina3221.getCurrent_mA(INA3221_CH2); 
}

float PowerManager::getCurrentChargeSolar() {
    if (!initialized) return 0.0f;
    return ina3221.getCurrent_mA(INA3221_CH3); 
}

float PowerManager::getCurrentChargeTotal() {
    if (!initialized) return 0.0f;
    return ina3221.getCurrent_mA(INA3221_CH1) + ina3221.getCurrent_mA(INA3221_CH3);
}

void PowerManager::configure(const std::map<std::string, std::string>& config) {
    if (!initialized) return;

    if (config.find("operating_mode") != config.end()) {
        if (config.at("operating_mode") == "continuous") {
            ina3221.setModeContinious();
        }
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
            default:
                ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
        }
    }
}

bool PowerManager::isSolarActive() const {
    if (!initialized) return false;
    return getCurrentChargeSolar() > SOLAR_CURRENT_THRESHOLD;
}

bool PowerManager::isUSBConnected() const {
    if (!initialized) return false;
    return getCurrentChargeUSB() > USB_CURRENT_THRESHOLD;
}