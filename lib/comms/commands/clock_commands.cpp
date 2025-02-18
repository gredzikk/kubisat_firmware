#include "communication.h"
#include <time.h>
#include "DS3231.h" // Include the DS3231 header

// Declare the systemClock as extern
extern DS3231 systemClock;


/**
 * @defgroup ClockCommands Clock Management Commands
 * @brief Commands for managing system time and clock settings
 * @{
 */

 extern DS3231 systemClock;

 /**
  * @brief Handler for getting and setting system time
  * @param param For SET: Unix timestamp as string, for GET: empty string
  * @param operationType GET/SET
  * @return Frame containing success/error and current time or confirmation
  * @details When getting time, returns format "HH:MM:SS Weekday DD.MM.YYYY"
  * @ingroup ClockCommands
  * @xrefitem command "Command" "Clock Commands" Command ID: 3.0
  */
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


/**
 * @brief Handler for getting and setting timezone offset
 * @param param For SET: Timezone offset in minutes (-720 to +720), for GET: empty string
 * @param operationType GET/SET
 * @return Frame containing success/error and timezone offset in minutes
 * @note GET: <b>KBST;0;GET;3;1;;KBST</b>
 * @note SET: <b>KBST;0;SET;3;1;OFFSET;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.1
 */
Frame handleTimezoneOffset(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return buildFrame(ExecutionResult::ERROR, 3, 1, "INVALID OPERATION");
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            return buildFrame(ExecutionResult::ERROR, 3, 1, "PARAM UNNECESSARY");
        }
        return buildFrame(ExecutionResult::SUCCESS, 3, 1,
                         std::to_string(systemClock.getTimezoneOffset()));
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            return buildFrame(ExecutionResult::ERROR, 3, 1, "PARAM REQUIRED");
        }
        try {
            int16_t offset = std::stoi(param);
            if (offset < -720 || offset > 720) { // ±12 hours in minutes
                return buildFrame(ExecutionResult::ERROR, 3, 1, "INVALID OFFSET");
            }
            systemClock.setTimezoneOffset(offset);
            return buildFrame(ExecutionResult::SUCCESS, 3, 1, param);
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 3, 1, "INVALID PARAMETER");
        }
    }
    return buildFrame(ExecutionResult::ERROR, 3, 1, "UNKNOWN ERROR");
}


/**
 * @brief Handler for getting and setting clock synchronization interval
 * @param param For SET: Sync interval in seconds, for GET: empty string
 * @param operationType GET/SET
 * @return Frame containing success/error and sync interval in seconds
 * @note GET: <b>KBST;0;GET;3;3;;KBST</b>
 * @note SET: <b>KBST;0;SET;3;3;INTERVAL;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.3
 */
Frame handleClockSyncInterval(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return buildFrame(ExecutionResult::ERROR, 3, 2, "INVALID OPERATION");
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            return buildFrame(ExecutionResult::ERROR, 3, 2, "PARAM UNNECESSARY");
        }
        return buildFrame(ExecutionResult::SUCCESS, 3, 2,
                         std::to_string(systemClock.getClockSyncInterval()));
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            return buildFrame(ExecutionResult::ERROR, 3, 2, "PARAM REQUIRED");
        }
        try {
            uint32_t interval = std::stoul(param);
            systemClock.setClockSyncInterval(interval);
            return buildFrame(ExecutionResult::SUCCESS, 3, 2, param);
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 3, 2, "INVALID PARAMETER");
        }
    }
    return buildFrame(ExecutionResult::ERROR, 3, 2, "UNKNOWN ERROR");
}

/**
 * @brief Handler for getting last clock sync time
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing success/error and last sync time as Unix timestamp
 * @note <b>KBST;0;GET;3;7;;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.7
 */
Frame handleGetLastSyncTime(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 3, "INVALID REQUEST");
    }
    return buildFrame(ExecutionResult::SUCCESS, 3, 3, 
                     std::to_string(systemClock.getLastSyncTime()));
}
/** @} */ // end of ClockCommands group