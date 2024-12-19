#include "commands.h"
#include "LoRa/LoRa-RP2040.h"
#include <string>
#include "pin_config.h"

std::map<std::string, Command> commandMap = {
    {"get_time", Command::GET_TIME},
    {"get_voltage_battery", Command::GET_VOLTAGE_BATTERY},
    {"get_voltage_5v", Command::GET_VOLTAGE_5V},
    {"get_current_charge_usb", Command::GET_CURRENT_CHARGE_USB},
    {"get_current_charge_solar", Command::GET_CURRENT_CHARGE_SOLAR},
    {"get_current_charge_total", Command::GET_CURRENT_CHARGE_TOTAL},
    {"get_current_draw", Command::GET_CURRENT_DRAW},
    {"get_gps_power_status", Command::GET_GPS_POWER_STATUS},
    {"set_gps_power_status", Command::SET_GPS_POWER_STATUS},
    {"commands", Command::COMMANDS}
};


void logMessage(const std::string& message);

std::pair<std::string, std::string> parseCommand(const std::string& message) {
    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos) {
        return {
            message.substr(0, spacePos),
            message.substr(spacePos + 1)
        };
    }
    return {message, ""};
}

void handleGetTime() {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    std::string response = "Current time: " + std::to_string(currentTime) + " ms";
    sendMessage(response);
}

void handleGetVoltageBattery() {
    sendMessage("Battery voltage: 3.7V");
}

void handleGetVoltage5V() {
    sendMessage("5V Rail Voltage: 5.0V");
}

void handleGetCurrentChargeUSB() {
    sendMessage("USB Charge Current: 500mA");
}

void handleGetCurrentChargeSolar() {
    sendMessage("Solar Charge Current: 100mA");
}

void handleGetCurrentChargeTotal() {
    sendMessage("Total Charge Current: 600mA");
}

void handleGetCurrentDraw() {
    sendMessage("Current Draw: 300mA");
}

void handleGetGPSPowerStatus() {
    bool status = gpio_get(GPS_POWER_ENABLE);
    std::string statusStr = status ? "ON" : "OFF";
    sendMessage("GPS Power Status: " + statusStr);
}

void handleSetGPSPowerStatus(const std::string& param) {
    if (param.empty()) {
        sendMessage("Error: GPS power status parameter required (on/off)");
        return;
    }
    
    bool powerOn = (param == "on" || param == "1" || param == "true");
    gpio_put(GPS_POWER_ENABLE, powerOn);
    std::string status = powerOn ? "ON" : "OFF";
    sendMessage("GPS Power Status set to: " + status);
}

void handleCommands() {
    std::string response = "\nAvailable commands:\n";
    for (const auto& [cmd, _] : commandMap) {
        response += "- " + cmd + "\n";
    }
    sendMessage(response);
}

void handleUnknownCommand() {
    std::string response = "Unknown command.\nFor available commands type 'commands'";
    sendMessage(response);
}

void handleCommand(const std::string& message) {
    auto [cmd, param] = parseCommand(message);
    auto it = commandMap.find(cmd);
    if (it != commandMap.end()) {
        switch (it->second) {
            case Command::GET_TIME:
                handleGetTime();
                break;
            case Command::GET_VOLTAGE_BATTERY:
                handleGetVoltageBattery();
                break;
            case Command::GET_VOLTAGE_5V:
                handleGetVoltage5V();
                break;
            case Command::GET_CURRENT_CHARGE_USB:
                handleGetCurrentChargeUSB();
                break;
            case Command::GET_CURRENT_CHARGE_SOLAR:
                handleGetCurrentChargeSolar();
                break;
            case Command::GET_CURRENT_CHARGE_TOTAL:
                handleGetCurrentChargeTotal();
                break;
            case Command::GET_CURRENT_DRAW:
                handleGetCurrentDraw();
                break;
            case Command::GET_GPS_POWER_STATUS:
                handleGetGPSPowerStatus();
                break;
            case Command::SET_GPS_POWER_STATUS:
                handleSetGPSPowerStatus(param);
                break;
            case Command::COMMANDS:
                handleCommands();
                break;
            default:
                handleUnknownCommand();
                break;
        }
    } else {
        handleUnknownCommand();
    }
}