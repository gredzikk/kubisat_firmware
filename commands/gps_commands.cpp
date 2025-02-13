#include "communication.h"

Frame handleGPSPowerStatus(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        // Handle GET operation
        // Read the current GPS power status
        bool powerStatus = gpio_get(GPS_POWER_ENABLE_PIN); // Assuming GPS_POWER_PIN is defined
        return buildSuccessFrame(1, 7, 1, std::to_string(powerStatus), ""); // Return the power status as a string ("0" or "1")
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
            return buildErrorFrame(1, 7, 1, "INVALID PARAM");
        }
        return buildSuccessFrame(1, 7, 1, "GPS PWR = " + std::to_string(powerStatus), "");
    } else {
        return buildErrorFrame(1, 7, 1, "INVALID OPERATION");
    }
}

Frame handleEnableGPSTransparentMode(const std::string& param, OperationType operationType) {
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

        return buildSuccessFrame(1, 7, 2, "GPS UART BRIDGE EXIT", "");
    } else {
        uartPrint("GET operation not allowed for EnableGPSTransparentMode");
        return buildErrorFrame(1, 7, 2, "NOT ALLOWED");
    }
}

Frame handleGetGPSData(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        extern GPSData gps_data;
        std::string nmea_data = gps_data.getNMEAData();
        return buildSuccessFrame(1, 7, 3, nmea_data, "");
    } else {
        uartPrint("GET operation is only allowed");
        return buildErrorFrame(1, 7, 3, "NOT ALLOWED");
    }
}