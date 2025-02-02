#include "commands.h"
#include "LoRa/LoRa-RP2040.h"
#include <string>
#include "pin_config.h"
#include "PowerManager.h"

extern PowerManager powerManager;

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
};


/**
 * @brief Parses a command string into command and parameter.
 * @param message The full command message, e.g. "cmd param".
 * @return A pair where first is the command, second is the parameter.
 */
std::pair<std::string, std::string> parseCommand(const std::string& message)
{
    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos) {
        return {
            message.substr(0, spacePos),
            message.substr(spacePos + 1)
        };
    }
    return {message, ""};
}

/**
 * @brief Retrieves and sends the current time since system boot.
 */
void handleGetTime() {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    std::string response = "Current time: " + std::to_string(currentTime) + " ms";
    sendMessage(response);
}

/**
 * @brief Measures and sends the battery voltage.
 */
void handleGetVoltageBattery() {
    float voltage = powerManager.getVoltageBattery();
    sendMessage("Battery voltage: " + std::to_string(voltage) + " V");
}

/**
 * @brief Measures and sends the 5V rail voltage.
 */
void handleGetVoltage5V() {
    float voltage = powerManager.getVoltage5V();
    sendMessage("5V Rail Voltage: " + std::to_string(voltage) + " V");
}

/**
 * @brief Measures and sends the USB charge current.
 */
void handleGetCurrentChargeUSB() {
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    sendMessage("USB Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

/**
 * @brief Measures and sends the solar panel charge current.
 */
void handleGetCurrentChargeSolar() {
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    sendMessage("Solar Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

/**
 * @brief Measures and sends the total charge current.
 */
void handleGetCurrentChargeTotal() {
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    sendMessage("Total Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

/**
 * @brief Measures and sends the current draw from the system.
 */
void handleGetCurrentDraw() {
    float currentDraw = powerManager.getCurrentDraw();
    sendMessage("Current Draw: " + std::to_string(currentDraw) + " mA");
}

/**
 * @brief Reads and sends the GPS power status (ON/OFF).
 */
void handleGetGPSPowerStatus() {
    bool status = gpio_get(GPS_POWER_ENABLE);
    std::string statusStr = status ? "ON" : "OFF";
    sendMessage("GPS Power Status: " + statusStr);
}

/**
 * @brief Sets the GPS power status to ON or OFF.
 * @param param String containing "on", "off", "1", "0", or "true"/"false".
 */
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

/**
 * @brief Handles unknown commands by sending an error message.
 */
void handleUnknownCommand() {
    std::string response = "Unknown command";
    sendMessage(response);
}

/**
 * @brief Main dispatcher for handling incoming commands.
 * @param message The full message, including command and parameters.
 */
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
            default:
                handleUnknownCommand();
                break;
        }
    } else {
        handleUnknownCommand();
    }
}