#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>
#include <string>
#include "pico/mutex.h"
#include "storage.h"
#include "utils.h"

#define EVENT_BUFFER_SIZE 10
#define EVENT_LOG_FILE "/event_log.txt"

// Event Groups
enum class EventGroup : uint8_t {
    SYSTEM  = 0x00,
    POWER   = 0x01,
    COMMS   = 0x02,
    GPS     = 0x03,
    CLOCK   = 0x04
};

// System Events
enum class SystemEvent : uint8_t {
    BOOT          = 0x01,
    SHUTDOWN      = 0x02,
    WATCHDOG_RESET = 0x03,
    CORE1_START   = 0x04,
    CORE1_STOP    = 0x05
};

// Power Events
enum class PowerEvent : uint8_t {
    LOW_BATTERY      = 0x01,
    OVERCHARGE       = 0x02,
    POWER_FALLING    = 0x03,
    POWER_NORMAL     = 0x04,
    SOLAR_ACTIVE     = 0x05,
    SOLAR_INACTIVE   = 0x06,
    USB_CONNECTED    = 0x07,
    USB_DISCONNECTED = 0x08
};

// Communication Events
enum class CommsEvent : uint8_t {
    RADIO_INIT    = 0x01,
    RADIO_ERROR   = 0x02,
    MSG_RECEIVED  = 0x03,
    MSG_SENT      = 0x04,
    UART_ERROR    = 0x06
};

// GPS Events
enum class GPSEvent : uint8_t {
    LOCK                = 0x01,
    LOST                = 0x02,
    ERROR               = 0x03,
    POWER_ON            = 0x04,
    POWER_OFF           = 0x05,
    DATA_READY          = 0x06,
    PASS_THROUGH_START  = 0x07,
    PASS_THROUGH_END    = 0x08,
};

// Clock Events
enum class ClockEvent : uint8_t {
    CHANGED  = 0x01,
    GPS_SYNC = 0x02
};


/**
 * @class EventLog
 * @brief Represents a single event log entry.
 */
class EventLog {
public:
    uint16_t id;          ///< Sequence number
    uint32_t timestamp;   ///< Unix timestamp or system time
    uint8_t group;        ///< Event group identifier
    uint8_t event;        ///< Specific event identifier

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
            , nextEventId(0)
            , needsPersistence(false)
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
        EventLog events[EVENT_BUFFER_SIZE];  ///< Event buffer
        size_t eventCount;                   ///< Number of events in the buffer
        size_t writeIndex;                   ///< Index of the next event to be written
        mutex_t eventMutex;                   ///< Mutex for protecting the event buffer
        volatile uint16_t nextEventId;        ///< Next event ID
        bool needsPersistence;              ///< Flag indicating whether the events need to be saved to storage
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
        bool save_to_storage() override {
            if(!sd_card_mounted) {
                bool status = fs_init();
                if(!status) {
                    return false;
                }
            } 
            FILE *file = fopen(EVENT_LOG_FILE, "a");
            if (file) {
                for (size_t i = 0; i < eventCount; i++) {
                    fwrite(&events[i], sizeof(EventLog), 1, file);
                }
                fclose(file);
                needsPersistence = false;
                uart_print("Events saved to storage", VerbosityLevel::INFO);
                return true;
            }
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