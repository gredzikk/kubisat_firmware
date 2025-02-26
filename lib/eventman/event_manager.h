#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>
#include <string>
#include "pico/mutex.h"
#include "storage.h"
#include "utils.h"

#define EVENT_BUFFER_SIZE 1000
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
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                        "EventLog: id=%u, timestamp=%lu, group=%u, event=%u",
                        id, timestamp, group, event);
            return std::string(buffer);
        }
    } __attribute__((packed));


/**
 * @class EventManager
 * @brief Manages the event logging system.
 */
class EventManager {
    public:
        /**
         * @brief Constructor for the EventManager.
         * @details Initializes the event buffer, mutex, and other internal variables.
         */
        EventManager()
            : eventCount(0)
            , writeIndex(0)
            , eventsSinceFlush(0)  // Add eventsSinceFlush initialization
        {
            mutex_init(&eventMutex);
        }
    
        /**
         * @brief Virtual destructor for the EventManager.
         */
        virtual ~EventManager() = default;
    
        /**
         * @brief Initializes the EventManager.
         * @details Loads events from storage.
         */
        virtual void init() {
            load_from_storage();
        }
    
        /**
         * @brief Logs an event.
         * @param group The event group.
         * @param event The event identifier.
         */
        void log_event(uint8_t group, uint8_t event);
    
        /**
         * @brief Retrieves an event from the event buffer.
         * @param index The index of the event to retrieve.
         * @return A const reference to the EventLog at the specified index.
         */
        const EventLog& get_event(size_t index) const;
    
        /**
         * @brief Gets the number of events in the buffer.
         * @return The number of events in the buffer.
         */
        size_t get_event_count() const { return eventCount; }
    
        /**
         * @brief Saves the events to storage.
         * @return True if the events were successfully saved, false otherwise.
         */
        virtual bool save_to_storage() = 0;
    
        /**
         * @brief Loads the events from storage.
         * @return True if the events were successfully loaded, false otherwise.
         */
        virtual bool load_from_storage() = 0;   

        protected:
        /** @brief Event buffer */
        EventLog events[EVENT_BUFFER_SIZE];
        /** @brief Number of events in the buffer */
        size_t eventCount;
        /** @brief Index of the next event to be written */
        size_t writeIndex;
        /** @brief Mutex for protecting the event buffer */
        mutex_t eventMutex;
        /** @brief Static event ID counter */
        static uint16_t nextEventId;
        /** @brief Number of events since last flush to storage */
        size_t eventsSinceFlush;
    };
    

/**
 * @class EventManagerImpl
 * @brief Implementation of the EventManager class.
 */
class EventManagerImpl : public EventManager {
    public:
        /**
         * @brief Constructor for the EventManagerImpl.
         * @details Initializes the EventManagerImpl and calls the init method.
         */
        EventManagerImpl() {
            init(); // Safe to call virtual functions here
        }
    
        /**
         * @brief Saves the events to storage.
         * @return True if the events were successfully saved, false otherwise.
         * @details This method is not yet implemented.
         */
        public:
        bool save_to_storage() override {
            if(!sd_card_mounted) {
                bool status = fs_init();
                if(!status) {
                    return false;
                }
            } 
            
            FILE *file = fopen(EVENT_LOG_FILE, "a");
            if (file) {
                // Calculate start index for last EVENT_FLUSH_THRESHOLD events
                size_t startIdx = (writeIndex >= eventsSinceFlush) ? 
                    writeIndex - eventsSinceFlush : 
                    EVENT_BUFFER_SIZE - (eventsSinceFlush - writeIndex);

                // Write only the most recent batch of events
                for (size_t i = 0; i < eventsSinceFlush; i++) {
                    size_t idx = (startIdx + i) % EVENT_BUFFER_SIZE;
                    fprintf(file, "%u;%lu;%u;%u\n", 
                        events[idx].id,
                        events[idx].timestamp,
                        events[idx].group,
                        events[idx].event
                    );
                }
                fclose(file);
                uart_print("Events saved to storage", VerbosityLevel::INFO);
                return true;
            }
            return false;
        }
        
        /**
         * @brief Loads the events from storage.
         * @return True if the events were successfully loaded, false otherwise.
         * @details This method is not yet implemented.
         */
        bool load_from_storage() override {
            // TODO: Implement based on chosen storage (SD/EEPROM)
            return false;
        }
    };


/**
 * @brief Global instance of the EventManagerImpl class.
 */
extern EventManagerImpl eventManager;

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
        eventManager.log_event(
            static_cast<uint8_t>(group), 
            static_cast<uint8_t>(event)
        );
    }
};


/**
 * @brief Checks power statuses and triggers events based on voltage trends.
 * @param pm Reference to the PowerManager object.
 */
void check_power_events(PowerManager& pm);


#endif
/** @} */ // End of EventManagerGroup