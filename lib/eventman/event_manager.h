#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>
#include <string>
#include "pico/mutex.h"
#include "storage.h"
#include "utils.h"
#include "system_state_manager.h"


#define EVENT_BUFFER_SIZE 100
#define EVENT_FLUSH_THRESHOLD 10
#define EVENT_LOG_FILE "/event_log.csv"

/**
 * @file event_manager.h
 * @brief Manages the event logging system for the Kubisat firmware.
 * @defgroup EventManagerGroup Event Manager
 * @brief Classes and functions for managing event logging.
 * @{
 */


/**
 * @enum EventGroup
 * @brief Represents the group to which an event belongs.
 */
enum class EventGroup : uint8_t {
    /** @brief System events. */
    SYSTEM  = 0x00,
    /** @brief Power-related events. */
    POWER   = 0x01,
    /** @brief Communication-related events. */
    COMMS   = 0x02,
    /** @brief GPS-related events. */
    GPS     = 0x03,
    /** @brief Clock-related events. */
    CLOCK   = 0x04
};

/**
 * @enum SystemEvent
 * @brief Represents specific system events.
 */
enum class SystemEvent : uint8_t {
    /** @brief System boot event. */
    BOOT          = 0x01,
    /** @brief System shutdown event. */
    SHUTDOWN      = 0x02,
    /** @brief Watchdog reset event. */
    WATCHDOG_RESET = 0x03,
    /** @brief Core 1 start event. */
    CORE1_START   = 0x04,
    /** @brief Core 1 stop event. */
    CORE1_STOP    = 0x05
};

/**
 * @enum PowerEvent
 * @brief Represents specific power-related events.
 */
enum class PowerEvent : uint8_t {
    /** @brief Low battery event. */
    LOW_BATTERY      = 0x01,
    /** @brief Overcharge event. */
    OVERCHARGE       = 0x02,
    /** @brief Power falling event. */
    POWER_FALLING    = 0x03,
    /** @brief Power normal event. */
    POWER_NORMAL     = 0x04,
    /** @brief Solar charging active event. */
    SOLAR_ACTIVE     = 0x05,
    /** @brief Solar charging inactive event. */
    SOLAR_INACTIVE   = 0x06,
    /** @brief USB connected event. */
    USB_CONNECTED    = 0x07,
    /** @brief USB disconnected event. */
    USB_DISCONNECTED = 0x08
};

/**
 * @enum CommsEvent
 * @brief Represents specific communication-related events.
 */
enum class CommsEvent : uint8_t {
    /** @brief Radio initialization event. */
    RADIO_INIT    = 0x01,
    /** @brief Radio error event. */
    RADIO_ERROR   = 0x02,
    /** @brief Message received event. */
    MSG_RECEIVED  = 0x03,
    /** @brief Message sent event. */
    MSG_SENT      = 0x04,
    /** @brief UART error event. */
    UART_ERROR    = 0x06
};

/**
 * @enum GPSEvent
 * @brief Represents specific GPS-related events.
 */
enum class GPSEvent : uint8_t {
    /** @brief GPS lock acquired event. */
    LOCK                = 0x01,
    /** @brief GPS lock lost event. */
    LOST                = 0x02,
    /** @brief GPS error event. */
    ERROR               = 0x03,
    /** @brief GPS power on event. */
    POWER_ON            = 0x04,
    /** @brief GPS power off event. */
    POWER_OFF           = 0x05,
    /** @brief GPS data ready event. */
    DATA_READY          = 0x06,
    /** @brief GPS pass-through start event. */
    PASS_THROUGH_START  = 0x07,
    /** @brief GPS pass-through end event. */
    PASS_THROUGH_END    = 0x08
};


/**
 * @enum ClockEvent
 * @brief Represents specific clock-related events.
 */
enum class ClockEvent : uint8_t {
    /** @brief Clock changed event. */
    CHANGED  = 0x01,
    /** @brief Clock synchronized with GPS event. */
    GPS_SYNC = 0x02,
    /** @brief Sync interval but data not ready */
    GPS_SYNC_DATA_NOT_READY = 0x03
};


/**
 * @class EventLog
 * @brief Represents a single event log entry.
 */
class EventLog {
    public:
        /** @brief Sequence number */
        uint16_t id;
        /** @brief Unix timestamp or system time */
        uint32_t timestamp;
        /** @brief Event group identifier */
        uint8_t group;
        /** @brief Specific event identifier */
        uint8_t event;
    
        /**
         * @brief Converts the EventLog to a string representation.
         * @return A string representation of the EventLog.
         */
        std::string to_string() const {
            char buffer[256] = {0};
            snprintf(buffer, sizeof(buffer),
                        "EventLog: id=%u, timestamp=%lu, group=%u, event=%u",
                        id, timestamp, group, event);
            return std::string(buffer);
        }
    } __attribute__((packed));


/**
 * @class EventManager
 * @brief Singleton class managing the event logging system
 * @details Provides thread-safe event logging with circular buffer storage
 *          and persistent storage capabilities
 */
class EventManager {
private:
    /** @brief Pointer to singleton instance */
    static EventManager* instance;
    
    /** @brief Mutex for thread-safe instance access */
    static mutex_t instance_mutex;
    
    /** @brief Private constructor to enforce singleton pattern */
    EventManager();
    
    /** @brief Deleted copy constructor */
    EventManager(const EventManager&) = delete;
    
    /** @brief Deleted assignment operator */
    EventManager& operator=(const EventManager&) = delete;
    
    /** @brief Circular buffer for event storage */
    EventLog events[EVENT_BUFFER_SIZE];
    
    /** @brief Current number of events in buffer */
    size_t eventCount;
    
    /** @brief Current write position in circular buffer */
    size_t writeIndex;
    
    /** @brief Mutex for thread-safe event access */
    mutex_t eventMutex;
    
    /** @brief Next available event ID */
    static uint16_t nextEventId;
    
    /** @brief Counter for events since last storage flush */
    size_t eventsSinceFlush;

public:
    /**
     * @brief Gets the singleton instance
     * @return Reference to EventManager instance
     */
    static EventManager& get_instance();
    
    /**
     * @brief Initializes the event manager
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * @brief Logs a new event
     * @param group Event group identifier
     * @param event Event identifier
     */
    void log_event(uint8_t group, uint8_t event);
    
    /**
     * @brief Retrieves an event from the buffer
     * @param index Index of event to retrieve
     * @return Const reference to the event log entry
     */
    const EventLog& get_event(size_t index) const;
    
    /**
     * @brief Gets the current event count
     * @return Number of events in buffer
     */
    size_t get_event_count() const { return eventCount; }
    
    /**
     * @brief Saves events to persistent storage
     * @return true if save successful
     */
    bool save_to_storage();
};


/**
 * @class EventEmitter
 * @brief Provides a static method for emitting events.
 */
class EventEmitter {
    public:
        /**
         * @brief Emits an event.
         * @param group The event group.
         * @param event The event identifier.
         * @tparam T The type of the event identifier.
         */
    template<typename T>
    static void emit(EventGroup group, T event) {
        EventManager::get_instance().log_event(
            static_cast<uint8_t>(group),
            static_cast<uint8_t>(event)
        );
    }
};

#endif
/** @} */ // End of EventManagerGroup