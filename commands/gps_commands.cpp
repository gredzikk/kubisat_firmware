#include "communication.h"

Frame handleGPSPowerStatus(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 1, "INVALID OPERATION");
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            return buildFrame(ExecutionResult::ERROR, 7, 1, "PARAM REQUIRED");
        }

        try {
            int powerStatus = std::stoi(param);
            if (powerStatus != 0 && powerStatus != 1) {
                return buildFrame(ExecutionResult::ERROR, 7, 1, "INVALID VALUE. USE 0 OR 1");
            }
            gpio_put(GPS_POWER_ENABLE_PIN, powerStatus);
            return buildFrame(ExecutionResult::SUCCESS, 7, 1, std::to_string(powerStatus));
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 7, 1, "INVALID PARAMETER FORMAT");
        }
    }

    // GET operation
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 1, "PARAM UNNECESSARY");
    }

    bool powerStatus = gpio_get(GPS_POWER_ENABLE_PIN);
    return buildFrame(ExecutionResult::SUCCESS, 7, 1, std::to_string(powerStatus));
}

Frame handleEnableGPSTransparentMode(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::SET)) {
        uartPrint("GET operation not allowed for EnableGPSTransparentMode");
        return buildFrame(ExecutionResult::ERROR, 7, 2, "NOT ALLOWED");
    }

    uartPrint("Enabling GPS Serial Pass-Through Mode, timeout " + param + "s");
    uint32_t timeoutMs;
    try {
        timeoutMs = param.empty() ? 60000u : std::stoul(param) * 1000;
    } catch (...) {
        return buildFrame(ExecutionResult::ERROR, 7, 2, "INVALID TIMEOUT FORMAT");
    }

    uint32_t originalBaudRate = DEBUG_UART_BAUD_RATE;
    uint32_t gpsBaudRate = GPS_UART_BAUD_RATE;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());

    std::string message = "Entering GPS Serial Pass-Through Mode @" + 
                         std::to_string(gpsBaudRate) + " for " + 
                         std::to_string(timeoutMs);
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
    return buildFrame(ExecutionResult::SUCCESS, 7, 2, "GPS UART BRIDGE EXIT");
}

Frame handleGetGPSData(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 3, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 3, "NOT ALLOWED");
    }

    extern GPSData gps_data;
    std::string nmea = gps_data.getNMEAData();
    return buildFrame(ExecutionResult::SUCCESS, 7, 3, nmea);
}