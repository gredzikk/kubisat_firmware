#include "protocol.h"
#include "LoRa-RP2040.h"
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>

void handleGetTime(const std::string& param) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    sendMessage("Current time: " + std::to_string(currentTime) + " ms");
}

void handleGetVoltageBattery(const std::string& param) {
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltageBattery();
    sendMessage("Battery voltage: " + std::to_string(voltage) + " V");
}

void handleGetVoltage5V(const std::string& param) {
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltage5V();
    sendMessage("5V Rail Voltage: " + std::to_string(voltage) + " V");
}

void handleGetCurrentChargeUSB(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    sendMessage("USB Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentChargeSolar(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    sendMessage("Solar Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentChargeTotal(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    sendMessage("Total Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentDraw(const std::string& param) {
    extern PowerManager powerManager;
    float currentDraw = powerManager.getCurrentDraw();
    sendMessage("Current Draw: " + std::to_string(currentDraw) + " mA");
}

void handleGetGPSPowerStatus(const std::string& param) {
    bool status = gpio_get(GPS_POWER_ENABLE_PIN);
    sendMessage("GPS Power Status: " + std::string(status ? "ON" : "OFF"));
}

void handleSetGPSPowerStatus(const std::string& param) {
    if (param.empty()) {
        sendMessage("Error: GPS power status parameter required (on/off)");
        return;
    }
    bool powerOn = (param == "on" || param == "1" || param == "true");
    gpio_put(GPS_POWER_ENABLE_PIN, powerOn);
    sendMessage("GPS Power Status set to: " + std::string(powerOn ? "ON" : "OFF"));
}

void handleEnableGPSTransparentMode(const std::string& param) {
    uint32_t timeoutMs = param.empty() ? 60000u : std::stoul(param) * 1000;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    sendMessage("Entering GPS Serial Pass-Through Mode. Type 'exit' to quit.");

    while (true) {
        while (uart_is_readable(DEBUG_UART_PORT)) {
            char ch = uart_getc(DEBUG_UART_PORT);
            uart_write_blocking(GPS_UART_PORT, reinterpret_cast<const uint8_t*>(&ch), 1);
        }
        while (uart_is_readable(GPS_UART_PORT)) {
            char gpsByte = uart_getc(GPS_UART_PORT);
            uart_write_blocking(DEBUG_UART_PORT, reinterpret_cast<const uint8_t*>(&gpsByte), 1);
        }
        if (to_ms_since_boot(get_absolute_time()) - startTime >= timeoutMs)
            break;
    }
    sendMessage("Exiting GPS Serial Pass-Through Mode.");
}


using CommandHandler = std::function<void(const std::string&)>;

using CommandMap = std::map<uint32_t, CommandHandler>;

CommandMap commandHandlers = {
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)), handleGetVoltageBattery}, // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(3)), handleGetVoltage5V}, // Group 2, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(4)), handleGetCurrentChargeUSB}, // Group 2, Command 4
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(5)), handleGetCurrentChargeSolar}, // Group 2, Command 5
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(6)), handleGetCurrentChargeTotal}, // Group 2, Command 6
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(7)), handleGetCurrentDraw}, // Group 2, Command 7
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)), handleGetTime},  // Group 3, Command 0
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(1)), handleGetGPSPowerStatus}, // Group 7, Command 1
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(2)), handleSetGPSPowerStatus}, // Group 7, Command 2
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(3)), handleEnableGPSTransparentMode}, // Group 7, Command 3
};

std::vector<uint8_t> executeCommand(uint32_t commandKey, const std::string& param) {
    std::vector<uint8_t> responseData;

    if (commandKey == (((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)))) {
        extern PowerManager powerManager;
        float voltage = powerManager.getVoltageBattery();
        std::string voltageStr = std::to_string(voltage);
        responseData.assign(voltageStr.begin(), voltageStr.end());
    }
    else if (commandKey == (((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)))) {
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        std::string timeStr = std::to_string(currentTime);
        responseData.assign(timeStr.begin(), timeStr.end());
    }
    else {
        std::string message = "Command executed successfully";
        responseData.assign(message.begin(), message.end());
    }

    return responseData;
}