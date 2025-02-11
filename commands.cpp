#include "protocol.h"
#include "LoRa-RP2040.h"
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <cstring>

// Modify handle functions to return vector<uint8_t>
std::vector<uint8_t> handleGetTime(const std::string& param) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    uint8_t timeBytes[sizeof(uint32_t)];
    std::memcpy(timeBytes, &currentTime, sizeof(uint32_t));
    return std::vector<uint8_t>(timeBytes, timeBytes + sizeof(uint32_t));
}

std::vector<uint8_t> handleGetVoltageBattery(const std::string& param) {
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltageBattery();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &voltage, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetVoltage5V(const std::string& param) {
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltage5V();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &voltage, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetCurrentChargeUSB(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &chargeCurrent, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetCurrentChargeSolar(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &chargeCurrent, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetCurrentChargeTotal(const std::string& param) {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &chargeCurrent, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetCurrentDraw(const std::string& param) {
    extern PowerManager powerManager;
    float currentDraw = powerManager.getCurrentDraw();
    uint8_t floatBytes[sizeof(float)];
    std::memcpy(floatBytes, &currentDraw, sizeof(float));
    return std::vector<uint8_t>(floatBytes, floatBytes + sizeof(float));
}

std::vector<uint8_t> handleGetGPSPowerStatus(const std::string& param) {
    bool status = gpio_get(GPS_POWER_ENABLE_PIN);
    std::string statusStr = status ? "ON" : "OFF";
    return std::vector<uint8_t>(statusStr.begin(), statusStr.end());
}

std::vector<uint8_t> handleSetGPSPowerStatus(const std::string& param) {
    if (param.empty()) {
        std::string message = "Error: GPS power status parameter required (on/off)";
        return std::vector<uint8_t>(message.begin(), message.end());
    }
    bool powerOn = (param == "on" || param == "1" || param == "true");
    gpio_put(GPS_POWER_ENABLE_PIN, powerOn);
    std::string message = "GPS Power Status set to: " + std::string(powerOn ? "ON" : "OFF");
    return std::vector<uint8_t>(message.begin(), message.end());
}

std::vector<uint8_t> handleEnableGPSTransparentMode(const std::string& param) {
    uint32_t timeoutMs = param.empty() ? 60000u : std::stoul(param) * 1000;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());

    // Store the original baud rate of the debug UART
    uint32_t originalBaudRate = DEBUG_UART_BAUD_RATE;

    // Set the baud rate of the debug UART to match the GPS UART
    uint32_t gpsBaudRate = GPS_UART_BAUD_RATE;
    std::string message = "Entering GPS Serial Pass-Through Mode @" + std::to_string(gpsBaudRate) + " for " + std::to_string(timeoutMs);

    uart_set_baudrate(DEBUG_UART_PORT, gpsBaudRate);

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

    // Restore the original baud rate of the debug UART
    uart_set_baudrate(DEBUG_UART_PORT, originalBaudRate);

    message = "Exiting GPS Serial Pass-Through Mode.";
    return std::vector<uint8_t>(message.begin(), message.end());
}


using CommandHandler = std::function<std::vector<uint8_t>(const std::string&)>;

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
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        return it->second(param); // Call the handler function
    } else {
        std::string message = "Unknown command";
        return std::vector<uint8_t>(message.begin(), message.end());
    }
}