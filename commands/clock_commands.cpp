#include "communication.h"
#include <time.h>
#include "DS3231.h" // Include the DS3231 header

// Declare the systemClock as extern
extern DS3231 systemClock;

Frame handleTime(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::SET && param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "PARAM REQUIRED");
    }

    if (operationType == OperationType::GET && !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "INVALID OPERATION");
    }

    if (operationType == OperationType::SET) {
        try {
            time_t newTime = std::stoll(param);
            struct tm* timeinfo = localtime(&newTime);

            if (timeinfo != nullptr) {
                // Set the time on the RTC
                systemClock.setTime(timeinfo->tm_sec, timeinfo->tm_min, timeinfo->tm_hour,
                                    timeinfo->tm_wday, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                EventEmitter::emit(EventGroup::CLOCK, ClockEvent::CHANGED);
                return buildFrame(ExecutionResult::SUCCESS, 3, 0, "Time set successfully");
            } else {
                return buildFrame(ExecutionResult::ERROR, 3, 0, "FAILED TO SET TIME");
            }
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 3, 0, "INVALID TIME FORMAT");
        }
    }

    // GET operation
    uint8_t sec, min, hour, day, month;
    uint16_t year;
    std::string weekday;
    if (systemClock.getTime(sec, min, hour, weekday, day, month, year)) {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(hour) << ":"
           << std::setw(2) << std::setfill('0') << static_cast<int>(min) << ":"
           << std::setw(2) << std::setfill('0') << static_cast<int>(sec) << " "
           << weekday << " "
           << std::setw(2) << std::setfill('0') << static_cast<int>(day) << "."
           << std::setw(2) << std::setfill('0') << static_cast<int>(month) << "."
           << year;
        return buildFrame(ExecutionResult::SUCCESS, 3, 0, ss.str());
    } else {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "FAILED TO GET TIME");
    }
}

Frame handleGetTimezoneOffset(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 1, "INVALID REQUEST");
    }
    return buildFrame(ExecutionResult::SUCCESS, 3, 1, 
                     std::to_string(systemClock.getTimezoneOffset()));
}

Frame handleSetTimezoneOffset(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::SET || param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 2, "INVALID REQUEST");
    }
    try {
        int16_t offset = std::stoi(param);
        if (offset < -720 || offset > 720) { // ±12 hours in minutes
            return buildFrame(ExecutionResult::ERROR, 3, 2, "INVALID OFFSET");
        }
        systemClock.setTimezoneOffset(offset);
        return buildFrame(ExecutionResult::SUCCESS, 3, 2, param);
    } catch (...) {
        return buildFrame(ExecutionResult::ERROR, 3, 2, "INVALID PARAMETER");
    }
}

Frame handleGetClockSyncInterval(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 3, "INVALID REQUEST");
    }
    return buildFrame(ExecutionResult::SUCCESS, 3, 3, 
                     std::to_string(systemClock.getClockSyncInterval()));
}

Frame handleSetClockSyncInterval(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::SET || param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 4, "INVALID REQUEST");
    }
    try {
        uint32_t interval = std::stoul(param);
        systemClock.setClockSyncInterval(interval);
        return buildFrame(ExecutionResult::SUCCESS, 3, 4, param);
    } catch (...) {
        return buildFrame(ExecutionResult::ERROR, 3, 4, "INVALID PARAMETER");
    }
}

Frame handleGetClockDrift(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 5, "INVALID REQUEST");
    }
    return buildFrame(ExecutionResult::SUCCESS, 3, 5, 
                     std::to_string(systemClock.getClockDrift()));
}

Frame handleSetClockDrift(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::SET || param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 6, "INVALID REQUEST");
    }
    try {
        float drift = std::stof(param);
        systemClock.setClockDrift(drift);
        return buildFrame(ExecutionResult::SUCCESS, 3, 6, param);
    } catch (...) {
        return buildFrame(ExecutionResult::ERROR, 3, 6, "INVALID PARAMETER");
    }
}

Frame handleGetLastSyncTime(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 7, "INVALID REQUEST");
    }
    return buildFrame(ExecutionResult::SUCCESS, 3, 7, 
                     std::to_string(systemClock.getLastSyncTime()));
}