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

enum class EventGroup : uint8_t {
    SYSTEM = 0x00,
    POWER = 0x01,
    COMMS = 0x02,
    GPS = 0x03,
    CLOCK = 0x04
};

enum class SystemEvent : uint8_t {
    BOOT = 0x01,
    SHUTDOWN = 0x02,
    WATCHDOG_RESET = 0x03,
    CORE1_START = 0x04,
    CORE1_STOP = 0x05
};

enum class PowerEvent : uint8_t {
    LOW_BATTERY = 0x01,
    OVERCHARGE = 0x02,
    POWER_FALLING = 0x03,
    POWER_NORMAL = 0x04,
    SOLAR_ACTIVE = 0x05,
    SOLAR_INACTIVE = 0x06,
    USB_CONNECTED = 0x07,
    USB_DISCONNECTED = 0x08
};

enum class CommsEvent : uint8_t {
    RADIO_INIT = 0x01,
    RADIO_ERROR = 0x02,
    MSG_RECEIVED = 0x03,
    MSG_SENT = 0x04,
    UART_ERROR = 0x06
};

enum class GPSEvent : uint8_t {
    LOCK = 0x01,
    LOST = 0x02,
    ERROR = 0x03,
    POWER_ON = 0x04,
    POWER_OFF = 0x05,
    DATA_READY = 0x06,
    PASS_THROUGH_START = 0x07,
    PASS_THROUGH_END = 0x08
};

enum class ClockEvent : uint8_t {
    CHANGED = 0x01,
    GPS_SYNC = 0x02,
    GPS_SYNC_DATA_NOT_READY = 0x03
};

class EventLog {
public:
    uint16_t id;
    uint32_t timestamp;
    uint8_t group;
    uint8_t event;
} __attribute__((packed));

class EventManager {
private:
    EventLog events[EVENT_BUFFER_SIZE];
    size_t eventCount;
    size_t writeIndex;
    mutex_t eventMutex;
    uint16_t nextEventId;
    size_t eventsSinceFlush;

    EventManager() :
        eventCount(0),
        writeIndex(0),
        nextEventId(0),
        eventsSinceFlush(0)
    {
        mutex_init(&eventMutex);
    }

    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

public:
    static EventManager& get_instance() {
        static EventManager instance;
        return instance;
    }

    bool init();
    void log_event(uint8_t group, uint8_t event);
    const EventLog& get_event(size_t index) const;
    size_t get_event_count() const { return eventCount; }
    bool save_to_storage();
    bool load_from_storage();
};

class EventEmitter {
public:
    template<typename T>
    static void emit(EventGroup group, T event) {
        EventManager::get_instance().log_event(
            static_cast<uint8_t>(group),
            static_cast<uint8_t>(event)
        );
    }
};

#endif