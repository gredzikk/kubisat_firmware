#include "communication.h"
#include "lib/GPS/gps_collector.h"
#include <sstream> // Include for stringstream

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

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::stringstream ss;
    ss << "Time: " << gpsData.time
       << ", Lat: " << gpsData.latitude << gpsData.latitudeDirection
       << ", Lon: " << gpsData.longitude << gpsData.longitudeDirection
       << ", Speed: " << gpsData.speedOverGround
       << ", Course: " << gpsData.courseOverGround
       << ", Date: " << gpsData.date;

    return buildFrame(ExecutionResult::SUCCESS, 7, 3, ss.str());
}

Frame handleGetGPSTime(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 4, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 4, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    return buildFrame(ExecutionResult::SUCCESS, 7, 4, gpsData.time);
}

Frame handleGetGPSLatitude(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 5, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 5, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << gpsData.latitude;
    return buildFrame(ExecutionResult::SUCCESS, 7, 5, ss.str());
}

Frame handleGetGPSLatitudeDirection(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 6, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 6, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::string direction(1, gpsData.latitudeDirection); // Convert char to string
    return buildFrame(ExecutionResult::SUCCESS, 7, 6, direction);
}

Frame handleGetGPSLongitude(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 7, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 7, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << gpsData.longitude;
    return buildFrame(ExecutionResult::SUCCESS, 7, 7, ss.str());
}

Frame handleGetGPSLongitudeDirection(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 8, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 8, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::string direction(1, gpsData.longitudeDirection); // Convert char to string
    return buildFrame(ExecutionResult::SUCCESS, 7, 8, direction);
}

Frame handleGetGPSSpeedOverGround(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 9, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 9, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << gpsData.speedOverGround;
    return buildFrame(ExecutionResult::SUCCESS, 7, 9, ss.str());
}

Frame handleGetGPSCourseOverGround(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 10, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 10, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << gpsData.courseOverGround;
    return buildFrame(ExecutionResult::SUCCESS, 7, 10, ss.str());
}

Frame handleGetGPSDate(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 7, 11, "NOT ALLOWED");
    }
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 7, 11, "PARAM UNNECESSARY");
    }

    ParsedGPSData gpsData = nmea_data.getParsedData();
    return buildFrame(ExecutionResult::SUCCESS, 7, 11, gpsData.date);
}