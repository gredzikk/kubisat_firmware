#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>

enum class PowerEventState {
    NORMAL,
    LOW_BATTERY,
    OVERCHARGE,
    POWER_FALLING
};

// Define event flags
enum class EventFlag : uint16_t {
    NONE = 0x0000,
    LOW_BATTERY = 0x0001,
    OVERCHARGE = 0x0002,
    POWER_FALLING = 0x0004
    // Add more events as needed
};

extern volatile uint16_t eventRegister;

void checkPowerEvents(PowerManager& pm);
void logEvent(const char* message);

#endif