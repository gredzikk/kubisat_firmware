#include "communication.h"
#include <time.h>
#include "DS3231.h" // Include the DS3231 header

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
  * @details 
  * @note GET: <b>KBST;0;GET;3;0;;KBST</b>
  * @note When getting time, returns format "HH:MM:SS Weekday DD.MM.YYYY"
  * @note SET: <b>KBST;0;SET;3;0;TIMESTAMP;KBST</b>
  * @note When setting time, expects Unix timestamp as parameter
  * @ingroup ClockCommands
  * @xrefitem command "Command" "Clock Commands" Command ID: 3.0
  */
Frame handle_time(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::SET && param.empty()) {
        return frame_build(ExecutionResult::ERROR, 3, 0, "PARAM REQUIRED");
    }

    if (operationType == OperationType::GET && !param.empty()) {
        return frame_build(ExecutionResult::ERROR, 3, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return frame_build(ExecutionResult::ERROR, 3, 0, "INVALID OPERATION");
    }

    if (operationType == OperationType::SET) {
        try {
            time_t newTime = std::stoll(param);
            if (newTime > 0) {
                uart_print("Setting time to: " + std::to_string(newTime));
                int status = systemClock.set_unix_time(newTime);
                if (status == 0) {
                    time_t time_after_set = systemClock.get_unix_time();
                    EventEmitter::emit(EventGroup::CLOCK, ClockEvent::CHANGED);
                    return frame_build(ExecutionResult::SUCCESS, 3, 0, std::to_string(time_after_set));
                } else {
                    return frame_build(ExecutionResult::ERROR, 3, 0, "FAILED TO SET TIME");
                }
            } else {
                return frame_build(ExecutionResult::ERROR, 3, 0, "TIME CANNOT BE 0");
            }
        } catch (...) {
            return frame_build(ExecutionResult::ERROR, 3, 0, "INVALID TIME FORMAT");
        }
    }

    uint32_t timeUnix = systemClock.get_unix_time();
    if (timeUnix != 0) {
        std::stringstream ss;
        ss << timeUnix;
        return frame_build(ExecutionResult::SUCCESS, 3, 0, ss.str());
    } else {
        return frame_build(ExecutionResult::ERROR, 3, 0, "FAILED TO GET TIME");
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
Frame handle_timezone_offset(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return frame_build(ExecutionResult::ERROR, 3, 1, "INVALID OPERATION");
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            return frame_build(ExecutionResult::ERROR, 3, 1, "PARAM UNNECESSARY");
        }

        std::string timezoneOffset = "60";
        return frame_build(ExecutionResult::SUCCESS, 3, 1, timezoneOffset);
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            return frame_build(ExecutionResult::ERROR, 3, 1, "PARAM REQUIRED");
        }
        try {
            int16_t offset = std::stoi(param);
            if (offset < -720 || offset > 720) { // ±12 hours in minutes
                return frame_build(ExecutionResult::ERROR, 3, 1, "INVALID OFFSET");
            }

            // set offset
            return frame_build(ExecutionResult::SUCCESS, 3, 1, param);
        } catch (...) {
            return frame_build(ExecutionResult::ERROR, 3, 1, "INVALID PARAMETER");
        }
    }
    return frame_build(ExecutionResult::ERROR, 3, 1, "UNKNOWN ERROR");
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
Frame handle_clock_sync_interval(const std::string& param, OperationType operationType) {
    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return frame_build(ExecutionResult::ERROR, 3, 2, "INVALID OPERATION");
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            return frame_build(ExecutionResult::ERROR, 3, 2, "PARAM UNNECESSARY");
        }

        std::string clockSyncInterval = "1440";
        return frame_build(ExecutionResult::SUCCESS, 3, 2, clockSyncInterval);
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            return frame_build(ExecutionResult::ERROR, 3, 2, "PARAM REQUIRED");
        }
        try {
            uint32_t interval = std::stoul(param);
            
            //set sync interval

            return frame_build(ExecutionResult::SUCCESS, 3, 2, param);
        } catch (...) {
            return frame_build(ExecutionResult::ERROR, 3, 2, "INVALID PARAMETER");
        }
    }
    return frame_build(ExecutionResult::ERROR, 3, 2, "UNKNOWN ERROR");
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
Frame handle_get_last_sync_time(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return frame_build(ExecutionResult::ERROR, 3, 3, "INVALID REQUEST");
    }
    std::string lastSyncTime = "none";
    return frame_build(ExecutionResult::SUCCESS, 3, 3, lastSyncTime);
}
/** @} */ // end of ClockCommands group