#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>
#include <string>
#include "pico/mutex.h"

#define EVENT_BUFFER_SIZE 1000

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


class EventLog {
public:
    uint16_t id;          // Sequence number
    uint32_t timestamp;   // Unix timestamp or system time
    uint8_t group;        // Event group identifier
    uint8_t event;        // Specific event identifier

    std::string toString() const {
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
                    "EventLog: id=%u, timestamp=%lu, group=%u, event=%u",
                    id, timestamp, group, event);
        return std::string(buffer);
    }
} __attribute__((packed));
    
class EventManager {
public:
        EventManager() 
        : eventCount(0)
        , writeIndex(0)
        , nextEventId(0)
        , needsPersistence(false) 
    {
        mutex_init(&eventMutex);
    }

    virtual ~EventManager() = default;

    // Add initialization method
    virtual void init() {
        loadFromStorage();
    }
    
    void logEvent(uint8_t group, uint8_t event);
    const EventLog& getEvent(size_t index) const;
    size_t getEventCount() const { return eventCount; }
    
    virtual bool saveToStorage() = 0;
    virtual bool loadFromStorage() = 0;

protected:
    EventLog events[EVENT_BUFFER_SIZE];
    size_t eventCount;
    size_t writeIndex;
    mutex_t eventMutex;
    volatile uint16_t nextEventId;
    bool needsPersistence;
};
    
class EventManagerImpl : public EventManager {
public:
    EventManagerImpl() {
        init(); // Safe to call virtual functions here
    }

    bool saveToStorage() override {
        // TODO: Implement based on chosen storage (SD/EEPROM)
        needsPersistence = false;
        return true;
    }
    
    bool loadFromStorage() override {
        // TODO: Implement based on chosen storage (SD/EEPROM)
        return false;
    }
};

extern EventManagerImpl eventManager;

class EventEmitter {
public:
    template<typename T>
    static void emit(EventGroup group, T event) {
        eventManager.logEvent(
            static_cast<uint8_t>(group), 
            static_cast<uint8_t>(event)
        );
    }
};

void checkPowerEvents(PowerManager& pm);

#endif