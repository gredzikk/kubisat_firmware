#include "commands.h"
#include "LoRa/LoRa-RP2040.h"
#include <string>
#include "pin_config.h"
#include "PowerManager.h"

extern PowerManager powerManager;

std::map<std::string, CommandHandler> commandRegistry = {
    {"get_time", [](const CommandMessage& msg) {
        handleGetTime();
    }},
    {"get_voltage_battery", [](const CommandMessage& msg) {
        handleGetVoltageBattery();
    }},
    {"get_voltage_5v", [](const CommandMessage& msg) {
        handleGetVoltage5V();
    }},
    {"get_current_charge_usb", [](const CommandMessage& msg) {
        handleGetCurrentChargeUSB();
    }},
    {"get_current_charge_solar", [](const CommandMessage& msg) {
        handleGetCurrentChargeSolar();
    }},
    {"get_current_charge_total", [](const CommandMessage& msg) {
        handleGetCurrentChargeTotal();
    }},
    {"get_current_draw", [](const CommandMessage& msg) {
        handleGetCurrentDraw();
    }},
    {"get_gps_power_status", [](const CommandMessage& msg) {
        handleGetGPSPowerStatus();
    }},
    {"set_gps_power_status", [](const CommandMessage& msg) {
        handleSetGPSPowerStatus(msg.parameter);
    }},
    {"enable_gps_transparent_mode", [](const CommandMessage& msg) {
        handleEnableGPSTransparentMode(msg.parameter);
    }},
};


/**
 * @brief Parses a command string into command and parameter.
 * @param message The full command message, e.g. "cmd param".
 * @return A pair where first is the command, second is the parameter.
 */
void handleCommandMessage(const std::string& message) {
    CommandMessage commandMsg(message);
    auto it = commandRegistry.find(commandMsg.command);
    if(it != commandRegistry.end()) {
        it->second(commandMsg);
    } else {
        handleUnknownCommand(commandMsg);
    }
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
 * @brief Enable GPS transparent UART communication in order to read or configure using u-center or similar software
 * @param Timeout in [s] after which transparent mode is automatically disabled
 */
void handleEnableGPSTransparentMode(const std::string& timeout) {
    uint32_t timeoutMs = timeout.empty() ? 30000 : std::stoul(timeout);
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    sendMessage("Entering GPS Serial Pass-Through Mode. Type 'exit' followed by newline to quit.");

    while (true) {
        // Read from PC (UART_ID) and pass to GPS (GPS_UART)
        while (uart_is_readable(UART_ID)) {
            char ch = uart_getc(UART_ID);
            // Forward character to GPS
            uart_write_blocking(GPS_UART, reinterpret_cast<const uint8_t*>(&ch), 1);
        }
        
        // Read from GPS (GPS_UART) and pass to PC (UART_ID)
        while (uart_is_readable(GPS_UART)) {
            char gpsByte = uart_getc(GPS_UART);
            uart_write_blocking(UART_ID, reinterpret_cast<const uint8_t*>(&gpsByte), 1);
        }

        if (to_ms_since_boot(get_absolute_time()) - startTime >= timeoutMs) {
            break;
        }
    }
    sendMessage("Exiting GPS Serial Pass-Through Mode.");
}

/**
 * @brief Handles unknown commands by sending an error message.
 */
void handleUnknownCommand(const CommandMessage& commandMsg) {
    std::string response = "Unknown command: " + commandMsg.command;
    sendMessage(response);
}
