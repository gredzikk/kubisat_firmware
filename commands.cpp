#include "commands.h"
#include "communication.h"
#include "LoRa-RP2040.h"  
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <thread>
#include <chrono>
#include <cstdlib>



void handleGetTime() {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    sendMessage("Current time: " + std::to_string(currentTime) + " ms");
}

void handleGetVoltageBattery() {
    // Assuming powerManager is defined elsewhere.
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltageBattery();
    sendMessage("Battery voltage: " + std::to_string(voltage) + " V");
}

void handleGetVoltage5V() {
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltage5V();
    sendMessage("5V Rail Voltage: " + std::to_string(voltage) + " V");
}

void handleGetCurrentChargeUSB() {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    sendMessage("USB Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentChargeSolar() {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    sendMessage("Solar Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentChargeTotal() {
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    sendMessage("Total Charge Current: " + std::to_string(chargeCurrent) + " mA");
}

void handleGetCurrentDraw() {
    extern PowerManager powerManager;
    float currentDraw = powerManager.getCurrentDraw();
    sendMessage("Current Draw: " + std::to_string(currentDraw) + " mA");
}

void handleGetGPSPowerStatus() {
    bool status = gpio_get(GPS_POWER_ENABLE);
    sendMessage("GPS Power Status: " + std::string(status ? "ON" : "OFF"));
}

void handleSetGPSPowerStatus(const std::string& param) {
    if (param.empty()) {
        sendMessage("Error: GPS power status parameter required (on/off)");
        return;
    }
    bool powerOn = (param == "on" || param == "1" || param == "true");
    gpio_put(GPS_POWER_ENABLE, powerOn);
    sendMessage("GPS Power Status set to: " + std::string(powerOn ? "ON" : "OFF"));
}

void handleEnableGPSTransparentMode(const std::string& timeout) {
    uint32_t timeoutMs = timeout.empty() ? 30000u : std::stoul(timeout) * 1000;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    sendMessage("Entering GPS Serial Pass-Through Mode. Type 'exit' to quit.");
    
    while (true) {
        while(uart_is_readable(DEBUG_UART_PORT)) {
            char ch = uart_getc(DEBUG_UART_PORT);
            uart_write_blocking(GPS_UART, reinterpret_cast<const uint8_t*>(&ch), 1);
        }
        while(uart_is_readable(GPS_UART)) {
            char gpsByte = uart_getc(GPS_UART);
            uart_write_blocking(DEBUG_UART_PORT, reinterpret_cast<const uint8_t*>(&gpsByte), 1);
        }
        if(to_ms_since_boot(get_absolute_time()) - startTime >= timeoutMs)
            break;
    }
    sendMessage("Exiting GPS Serial Pass-Through Mode.");
}