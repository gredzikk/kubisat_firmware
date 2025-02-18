#include "PowerManager.h"
#include <iostream>

PowerManager::PowerManager(i2c_inst_t* i2c) 
    : ina3221(INA3221_ADDR40_GND, i2c) {
        recursive_mutex_init(&mutex);
    };

bool PowerManager::initialize() {
    recursive_mutex_enter_blocking(&mutex);
    initialized = ina3221.begin();
    recursive_mutex_exit(&mutex);
    return initialized;
}

std::string PowerManager::readIDs() {
    if (!initialized) return "noinit";
    recursive_mutex_enter_blocking(&mutex);
    std::string MAN = "MAN " + std::to_string(ina3221.getManufID());
    std::string DIE = "DIE " + std::to_string(ina3221.getDieID());
    recursive_mutex_exit(&mutex);
    return MAN + " - " + DIE;
}

float PowerManager::getVoltageBattery() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float voltage = ina3221.getVoltage(INA3221_CH1);
    recursive_mutex_exit(&mutex);
    return voltage;
}

float PowerManager::getVoltage5V() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float voltage = ina3221.getVoltage(INA3221_CH2);
    recursive_mutex_exit(&mutex);
    return voltage;
}

float PowerManager::getCurrentChargeUSB() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float current = ina3221.getCurrent_mA(INA3221_CH1);
    recursive_mutex_exit(&mutex);
    return current;
}

float PowerManager::getCurrentDraw() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float current = ina3221.getCurrent_mA(INA3221_CH2);
    recursive_mutex_exit(&mutex);
    return current;
}

float PowerManager::getCurrentChargeSolar() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float current = ina3221.getCurrent_mA(INA3221_CH3);
    recursive_mutex_exit(&mutex);
    return current;
}

float PowerManager::getCurrentChargeTotal() {
    if (!initialized) return 0.0f;
    recursive_mutex_enter_blocking(&mutex);
    float current = ina3221.getCurrent_mA(INA3221_CH1) + ina3221.getCurrent_mA(INA3221_CH3);
    recursive_mutex_exit(&mutex);
    return current;
}

void PowerManager::configure(const std::map<std::string, std::string>& config) {
    if (!initialized) return;
    recursive_mutex_enter_blocking(&mutex);

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
    recursive_mutex_exit(&mutex);
}

bool PowerManager::isSolarActive() {
    if (!initialized) return false;
    recursive_mutex_enter_blocking(&mutex);
    bool active = getCurrentChargeSolar() > SOLAR_CURRENT_THRESHOLD;
    recursive_mutex_exit(&mutex);
    return active;
}

bool PowerManager::isUSBConnected() {
    if (!initialized) return false;
    recursive_mutex_enter_blocking(&mutex);
    bool connected = getCurrentChargeUSB() > USB_CURRENT_THRESHOLD;
    recursive_mutex_exit(&mutex);
    return connected;
}