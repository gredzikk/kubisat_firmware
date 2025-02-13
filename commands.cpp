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
#include "time.h"
#include "gps_data.h"
#include "build_number.h"

#define EXCEPTION_NOT_ALLOWED "NOT ALLOWED"
#define EXCEPTION_INVALID_PARAM "INVALID PARAM"
#define EXCEPTION_INVALID_OPERATION "INVALID OPERATION"
#define EXCEPTION_PARAM_UNECESSARY "PARAM UNECESSARY"

std::string handleGetBuildVersion(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        throw std::runtime_error(EXCEPTION_PARAM_UNECESSARY);
    }
    if (operationType == OperationType::GET) {
        return std::to_string(BUILD_NUMBER);
    }
    throw std::runtime_error(EXCEPTION_INVALID_OPERATION);
}

std::string handleGetPowerManagerIDs(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        throw std::runtime_error(EXCEPTION_PARAM_UNECESSARY);
    }

    if (!(operationType == OperationType::GET)) {
        throw std::runtime_error(EXCEPTION_INVALID_OPERATION);
    }

    extern PowerManager powerManager;
    std::string powerManagerIDS = powerManager.readIDs();
    return powerManagerIDS;
}

std::string handleTime(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting current time");
        time_t currentTime;
        time(&currentTime); // Get current time in seconds since epoch
        return std::to_string(currentTime);
    } else if (operationType == OperationType::SET) {
        uartPrint("Setting current time");
        time_t newTime = std::stoll(param); // Convert parameter to time_t
        uartPrint("New time: " + std::to_string(newTime));
        return std::to_string(newTime) + "OK";
    } else {
        throw std::runtime_error(EXCEPTION_INVALID_OPERATION);
    }
}

std::string handleGetVoltageBattery(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting battery voltage");
        extern PowerManager powerManager;
        float voltage = powerManager.getVoltageBattery();
        return std::to_string(voltage);
    } else {
        uartPrint("SET operation not allowed for GetVoltageBattery");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetVoltage5V(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting 5V voltage");
        extern PowerManager powerManager;
        float voltage = powerManager.getVoltage5V();
        return std::to_string(voltage);
    } else {
        uartPrint("SET operation not allowed for GetVoltage5V");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetCurrentChargeUSB(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting USB charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeUSB();
        return std::to_string(chargeCurrent);
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeUSB");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetCurrentChargeSolar(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting solar charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeSolar();
        return std::to_string(chargeCurrent);
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeSolar");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetCurrentChargeTotal(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting total charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeTotal();
        return std::to_string(chargeCurrent);
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeTotal");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetCurrentDraw(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting current draw");
        extern PowerManager powerManager;
        float currentDraw = powerManager.getCurrentDraw();
        return std::to_string(currentDraw);
    } else {
        uartPrint("SET operation not allowed for GetCurrentDraw");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGPSPowerStatus(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        // Handle GET operation
        // Read the current GPS power status
        bool powerStatus = gpio_get(GPS_POWER_ENABLE_PIN); // Assuming GPS_POWER_PIN is defined
        return std::to_string(powerStatus); // Return the power status as a string ("0" or "1")
    } else if (operationType == OperationType::SET) {
        // Handle SET operation
        int powerStatus = std::stoi(param);
        if (powerStatus == 0) {
            uartPrint("Turning GPS OFF");
            gpio_put(GPS_POWER_ENABLE_PIN, powerStatus); // Assuming GPS_POWER_PIN is defined
        } else if (powerStatus == 1) {
            uartPrint("Turning GPS ON");
            gpio_put(GPS_POWER_ENABLE_PIN, powerStatus); // Assuming GPS_POWER_PIN is defined
        } else {
            throw std::runtime_error(EXCEPTION_INVALID_PARAM);
        }
        return "GPS PWR = " + std::to_string(powerStatus);
    } else {
        throw std::runtime_error(EXCEPTION_INVALID_OPERATION);
    }
}

std::string handleEnableGPSTransparentMode(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::SET) {
        uartPrint("Enabling GPS Serial Pass-Through Mode, timeout " + param + "s");
        uint32_t timeoutMs = param.empty() ? 60000u : std::stoul(param) * 1000;
        uint32_t startTime = to_ms_since_boot(get_absolute_time());

        uint32_t originalBaudRate = DEBUG_UART_BAUD_RATE;

        uint32_t gpsBaudRate = GPS_UART_BAUD_RATE;
        std::string message = "Entering GPS Serial Pass-Through Mode @" + std::to_string(gpsBaudRate) + " for " + std::to_string(timeoutMs);
        uartPrint(message);
        sleep_ms(10);
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

        uart_set_baudrate(DEBUG_UART_PORT, originalBaudRate);

        message = "GPS UART BRIDGE EXIT";
        return message;
    } else {
        uartPrint("GET operation not allowed for EnableGPSTransparentMode");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

std::string handleGetGPSData(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        extern GPSData gps_data;
        std::string nmea_data = gps_data.getNMEAData();
        return nmea_data;
    } else {
        uartPrint("GET operation is only allowed");
        throw std::runtime_error(EXCEPTION_NOT_ALLOWED);
    }
}

using CommandHandler = std::function<std::string(const std::string&, OperationType)>;
using CommandMap = std::map<uint32_t, CommandHandler>;

CommandMap commandHandlers = {
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(1)), handleGetBuildVersion},          // Group 1, Command 1
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(0)), handleGetPowerManagerIDs},        // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)), handleGetVoltageBattery},        // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(3)), handleGetVoltage5V},             // Group 2, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(4)), handleGetCurrentChargeUSB},      // Group 2, Command 4
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(5)), handleGetCurrentChargeSolar},    // Group 2, Command 5
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(6)), handleGetCurrentChargeTotal},    // Group 2, Command 6
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(7)), handleGetCurrentDraw},           // Group 2, Command 7
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)), handleTime},                     // Group 3, Command 0
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(1)), handleGPSPowerStatus},           // Group 7, Command 1
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(2)), handleEnableGPSTransparentMode}, // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(3)), handleGetGPSData},               // Group 7, Command 4
};

std::string executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType) {
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        CommandHandler handler = it->second;
        return handler(param, operationType);
    } else {
        return "Error: Unknown command";
    }
}