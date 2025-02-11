// commands.cpp
#include "protocol.h"
#include "LoRa-RP2040.h"
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <cstring>
#include "utils.h"

// Modify handle functions to return string
std::string handleGetTime(const std::string& param) {
    uartPrint("Getting current time");
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    return std::to_string(currentTime);
}

std::string handleGetVoltageBattery(const std::string& param) {
    uartPrint("Getting battery voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltageBattery();
    return std::to_string(voltage);
}

std::string handleGetVoltage5V(const std::string& param) {
    uartPrint("Getting 5V voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltage5V();
    return std::to_string(voltage);
}

std::string handleGetCurrentChargeUSB(const std::string& param) {
    uartPrint("Getting USB charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    return std::to_string(chargeCurrent);
}

std::string handleGetCurrentChargeSolar(const std::string& param) {
    uartPrint("Getting solar charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    return std::to_string(chargeCurrent);
}

std::string handleGetCurrentChargeTotal(const std::string& param) {
    uartPrint("Getting total charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    return std::to_string(chargeCurrent);
}

std::string handleGetCurrentDraw(const std::string& param) {
    uartPrint("Getting current draw");
    extern PowerManager powerManager;
    float currentDraw = powerManager.getCurrentDraw();
    return std::to_string(currentDraw);
}

std::string handleGetGPSPowerStatus(const std::string& param) {
    uartPrint("Getting GPS power status");
    bool status = gpio_get(GPS_POWER_ENABLE_PIN);
    return status ? "ON" : "OFF";
}

std::string handleSetGPSPowerStatus(const std::string& param) {
    uartPrint("Setting GPS power status to " + param);
    if (param.empty()) {
        return "Error: GPS power status parameter required (on/off)";
    }
    bool powerOn = (param == "on" || param == "1" || param == "true");
    gpio_put(GPS_POWER_ENABLE_PIN, powerOn);
    return "GPS Power Status set to: " + std::string(powerOn ? "ON" : "OFF");
}

std::string handleEnableGPSTransparentMode(const std::string& param) {
    uartPrint("Enabling GPS Serial Pass-Through Mode, timeout " + param + "s");
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
    return message;
}

using CommandHandler = std::function<std::string(const std::string&)>;

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

std::string executeCommand(uint32_t commandKey, const std::string& param) {
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        uartPrint("Executing command: " + std::to_string(commandKey));
        return it->second(param); // Call the handler function
    } else {
        return "Unknown command";
    }
}