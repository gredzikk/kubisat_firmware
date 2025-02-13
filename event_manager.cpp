#include "event_manager.h"
#include <cstdio>
#include "protocol.h"
#include "pico/multicore.h"
#include "communication.h"

extern PowerManager powerManager;

static PowerEventState lastPowerState = PowerEventState::LOW_BATTERY;

static constexpr float FALL_RATE_THRESHOLD = -0.02f;
static constexpr int FALLING_TREND_REQUIRED = 3;
static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;

static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f;
static uint32_t lastEventTime = 0;

static int fallingTrendCount = 0;

#define POWER_FALLING_SIGNAL 0x01 // Example: Use ASCII SOH (Start of Heading)
#define EVENT_LOG_SIGNAL 0x02 // Define a signal to indicate an event log message

// Initialize the event register
volatile uint16_t eventRegister = 0x0000;

/**
 * @brief Checks power statuses and triggers events based on voltage trends.
 * @param pm Reference to the PowerManager object.
 */
void checkPowerEvents(PowerManager& pm) {
    float currentVoltage = pm.getVoltage5V();

    static float previousVoltage = 0.0f;
    float delta = currentVoltage - previousVoltage;
    previousVoltage = currentVoltage;

    if (delta < FALL_RATE_THRESHOLD) {
        fallingTrendCount++;
    } else {
        fallingTrendCount = 0;
    }

    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    if (fallingTrendCount >= FALLING_TREND_REQUIRED) {
        lastPowerState = PowerEventState::POWER_FALLING;
        lastEventTime = currentTime;

        eventRegister |= static_cast<uint16_t>(EventFlag::POWER_FALLING);
        sendEventRegister(); 
    }

    if (currentVoltage < VOLTAGE_LOW_THRESHOLD && lastPowerState != PowerEventState::LOW_BATTERY) {
        lastPowerState = PowerEventState::LOW_BATTERY;
        lastEventTime = currentTime;

        eventRegister |= static_cast<uint16_t>(EventFlag::LOW_BATTERY);
        sendEventRegister(); // Send the event register value
    } 

    else if (currentVoltage > VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::OVERCHARGE) {
        lastPowerState = PowerEventState::OVERCHARGE;

        eventRegister |= static_cast<uint16_t>(EventFlag::OVERCHARGE);
        sendEventRegister(); // Send the event register value
    } 
    
    else if (currentVoltage >= VOLTAGE_LOW_THRESHOLD && currentVoltage <= VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::NORMAL) {
        lastPowerState = PowerEventState::NORMAL;
        lastEventTime = currentTime;
        sendEventRegister();
    }
}
