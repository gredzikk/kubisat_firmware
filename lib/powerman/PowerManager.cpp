#include "PowerManager.h"
#include <iostream>
#include <sstream>
#include "event_manager.h"

PowerManager::PowerManager() 
    : ina3221_(INA3221_ADDR40_GND, MAIN_I2C_PORT) {
    recursive_mutex_init(&powerman_mutex_);
}

PowerManager& PowerManager::get_instance() {
    static PowerManager instance;
    return instance;
}

bool PowerManager::initialize() {
    recursive_mutex_enter_blocking(&powerman_mutex_);
    initialized_ = ina3221_.begin();
    
    if (initialized_) {
        // Set up alerts
        ina3221_.set_warn_alert_limit(INA3221_CH2, VOLTAGE_LOW_THRESHOLD);
        ina3221_.set_crit_alert_limit(INA3221_CH2, VOLTAGE_OVERCHARGE_THRESHOLD);
        ina3221_.set_power_valid_limit(VOLTAGE_OVERCHARGE_THRESHOLD, VOLTAGE_LOW_THRESHOLD);
        ina3221_.enable_alerts();
        ina3221_.set_alert_latch(true);
    }
    
    recursive_mutex_exit(&powerman_mutex_);
    return initialized_;
}

std::string PowerManager::read_device_ids() {
    if (!initialized_) return "noinit";
    recursive_mutex_enter_blocking(&powerman_mutex_);
    std::stringstream man_ss;
    man_ss << std::hex << ina3221_.get_manufacturer_id();
    std::string MAN = "MAN 0x" + man_ss.str();

    std::stringstream die_ss;
    die_ss << std::hex << ina3221_.get_die_id();
    std::string DIE = "DIE 0x" + die_ss.str();
    recursive_mutex_exit(&powerman_mutex_);
    return MAN + " - " + DIE;
}

float PowerManager::get_voltage_battery() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float voltage = ina3221_.get_voltage(INA3221_CH1);
    recursive_mutex_exit(&powerman_mutex_);
    return voltage;
}

float PowerManager::get_voltage_5v() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float voltage = ina3221_.get_voltage(INA3221_CH2);
    recursive_mutex_exit(&powerman_mutex_);
    return voltage;
}

float PowerManager::get_current_charge_usb() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH1);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

float PowerManager::get_current_draw() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH2);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

float PowerManager::get_current_charge_solar() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH3);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

float PowerManager::get_current_charge_total() {
    if (!initialized_) return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH1) + ina3221_.get_current_ma(INA3221_CH3);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

void PowerManager::configure(const std::map<std::string, std::string>& config) {
    if (!initialized_) return;
    recursive_mutex_enter_blocking(&powerman_mutex_);

    if (config.find("operating_mode") != config.end()) {
        if (config.at("operating_mode") == "continuous") {
            ina3221_.set_mode_continuous();
        }
    }

    if (config.find("averaging_mode") != config.end()) {
        int avg_mode = std::stoi(config.at("averaging_mode"));
        switch(avg_mode) {
            case 1:
                ina3221_.set_averaging_mode(INA3221_REG_CONF_AVG_1);
                break;
            case 4:
                ina3221_.set_averaging_mode(INA3221_REG_CONF_AVG_4);
                break;
            case 16:
                ina3221_.set_averaging_mode(INA3221_REG_CONF_AVG_16);
                break;
            default:
                ina3221_.set_averaging_mode(INA3221_REG_CONF_AVG_16);
        }
    }
    recursive_mutex_exit(&powerman_mutex_);
}

bool PowerManager::is_charging_solar() {
    if (!initialized_) return false;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    bool active = get_current_charge_solar() > SOLAR_CURRENT_THRESHOLD;
    recursive_mutex_exit(&powerman_mutex_);
    return active;
}

bool PowerManager::is_charging_usb() {
    if (!initialized_) return false;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    bool connected = get_current_charge_usb() > USB_CURRENT_THRESHOLD;
    recursive_mutex_exit(&powerman_mutex_);
    return connected;
}

bool PowerManager::check_power_alerts() {
    if (!initialized_) return false;
    
    recursive_mutex_enter_blocking(&powerman_mutex_);
    
    bool status_changed = false;
    
    // Check warning alert (low battery)
    if (ina3221_.get_warn_alert(INA3221_CH2)) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::LOW_BATTERY);
        status_changed = true;
    }
    
    // Check critical alert (overcharge)
    if (ina3221_.get_crit_alert(INA3221_CH2)) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::OVERCHARGE);
        status_changed = true;
    }
    
    // Check power valid alert
    if (ina3221_.get_power_valid_alert()) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::POWER_NORMAL);
        status_changed = true;
    }
    
    recursive_mutex_exit(&powerman_mutex_);
    return status_changed;
}