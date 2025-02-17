#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>

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

struct EventLog {
    uint16_t id;          // Sequence number
    uint32_t timestamp;   // Unix timestamp or system time
    EventGroup group;     // Event group identifier
    uint8_t event;        // Specific event identifier
} __attribute__((packed)); // Ensure no padding

// Forward declare logEvent before EventEmitter class
void logEvent(uint8_t group, uint8_t event);

class EventEmitter {
public:
    template<typename T>
    static void emit(EventGroup group, T event) {
        logEvent(static_cast<uint8_t>(group), static_cast<uint8_t>(event));
    }
};

extern volatile uint16_t eventLogId;
void checkPowerEvents(PowerManager& pm);

#endif