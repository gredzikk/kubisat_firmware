#include "communication.h"
#include <time.h>
#include "DS3231.h" // Include the DS3231 header

#define CLOCK_GROUP 3
#define TIME 0
#define TIMEZONE_OFFSET 1
#define CLOCK_SYNC_INTERVAL 2
#define LAST_SYNC_TIME 3

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
 * @return Vector of frames containing success/error and current time or confirmation
 * @details 
 * @note GET: <b>KBST;0;GET;3;0;;KBST</b>
 * @note When getting time, returns format "HH:MM:SS Weekday DD.MM.YYYY"
 * @note SET: <b>KBST;0;SET;3;0;TIMESTAMP;KBST</b>
 * @note When setting time, expects Unix timestamp as parameter
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.0
 */
std::vector<Frame> handle_time(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    
    if (operationType == OperationType::SET) {
        if (param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "PARAM REQUIRED"));
            return frames;
        }
        try {
            time_t newTime = std::stoll(param);
            if (newTime <= 0) {
                frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "TIME MUST BE POSITIVE"));
                return frames;
            }

            if (systemClock.set_unix_time(newTime) != 0) {
                frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "FAILED TO SET TIME"));
                return frames;
            }

            EventEmitter::emit(EventGroup::CLOCK, ClockEvent::CHANGED);
            frames.push_back(frame_build(OperationType::RES, CLOCK_GROUP, TIME, std::to_string(systemClock.get_unix_time())));
            return frames;
        } catch (...) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "INVALID TIME FORMAT"));
            return frames;
        }
    } else if (operationType == OperationType::GET) {
        if (!param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "PARAM UNNECESSARY"));
            return frames;
        }

        uint32_t timeUnix = systemClock.get_unix_time();
        if (timeUnix == 0) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "FAILED TO GET TIME"));
            return frames;
        }

        frames.push_back(frame_build(OperationType::VAL, CLOCK_GROUP, TIME, std::to_string(timeUnix)));
        return frames;
    }

    frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIME, "INVALID OPERATION"));
    return frames;
}

/**
 * @brief Handler for getting and setting timezone offset
 * @param param For SET: Timezone offset in minutes (-720 to +720), for GET: empty string
 * @param operationType GET/SET
 * @return Vector of frames containing success/error and timezone offset in minutes
 * @note GET: <b>KBST;0;GET;3;1;;KBST</b>
 * @note SET: <b>KBST;0;SET;3;1;OFFSET;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.1
 */
std::vector<Frame> handle_timezone_offset(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIMEZONE_OFFSET, "INVALID OPERATION"));
        return frames;
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIMEZONE_OFFSET, "PARAM UNNECESSARY"));
            return frames;
        }

        frames.push_back(frame_build(OperationType::VAL, CLOCK_GROUP, TIMEZONE_OFFSET, "60"));
        return frames;
    }

    if (param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIMEZONE_OFFSET, "PARAM REQUIRED"));
        return frames;
    }

    try {
        int16_t offset = std::stoi(param);
        if (offset < -720 || offset > 720) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIMEZONE_OFFSET, "INVALID OFFSET"));
            return frames;
        }

        frames.push_back(frame_build(OperationType::RES, CLOCK_GROUP, TIMEZONE_OFFSET, param));
        return frames;
    } catch (...) {
        frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, TIMEZONE_OFFSET, "INVALID PARAMETER"));
        return frames;
    }
}


/**
 * @brief Handler for getting and setting clock synchronization interval
 * @param param For SET: Sync interval in seconds, for GET: empty string
 * @param operationType GET/SET
 * @return Vector with frame containing success/error and sync interval in seconds
 * @note GET: <b>KBST;0;GET;3;3;;KBST</b>
 * @note SET: <b>KBST;0;SET;3;3;INTERVAL;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.3
 */
std::vector<Frame> handle_clock_sync_interval(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, "INVALID OPERATION"));
        return frames;
    }

    if (operationType == OperationType::GET) {
        if (!param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, "PARAM UNNECESSARY"));
            return frames;
        }

        std::string clockSyncInterval = "1440";
        frames.push_back(frame_build(OperationType::VAL, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, clockSyncInterval));
        return frames;
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, "PARAM REQUIRED"));
            return frames;
        }
        try {
            uint32_t interval = std::stoul(param);
            
            //set sync interval

            frames.push_back(frame_build(OperationType::RES, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, param));
            return frames;
        } catch (...) {
            frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, "INVALID PARAMETER"));
        }
    }
    frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, CLOCK_SYNC_INTERVAL, "UNKNOWN ERROR"));
    return frames;
}

/**
 * @brief Handler for getting last clock sync time
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector with one frame containing success/error and last sync time as Unix timestamp
 * @note <b>KBST;0;GET;3;7;;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.7
 */
std::vector<Frame> handle_get_last_sync_time(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET || !param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, CLOCK_GROUP, LAST_SYNC_TIME, "INVALID REQUEST"));
        return frames;
    }
    std::string lastSyncTime = "none";
    frames.push_back(frame_build(OperationType::VAL, CLOCK_GROUP, LAST_SYNC_TIME, lastSyncTime));
    return frames;
}
/** @} */ // end of ClockCommands group