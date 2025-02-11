#include "event_manager.h"
#include "storage.h"
#include <cstdio>
#include "ff.h"
#include "protocol.h"
#include "pico/multicore.h"

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


/**
 * @brief Checks power statuses and triggers events based on voltage trends.
 * @param pm Reference to the PowerManager object.
 */
void checkPowerEvents(PowerManager& pm)
{
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

    if (fallingTrendCount >= FALLING_TREND_REQUIRED)
    {
        sendMessage("ALERT: POWER FALLING | Voltage: " + std::to_string(currentVoltage) + " V");
        lastPowerState = PowerEventState::POWER_FALLING;
        lastEventTime = currentTime;

        multicore_fifo_push_blocking(POWER_FALLING_SIGNAL);
    }

    if (currentVoltage < VOLTAGE_LOW_THRESHOLD && lastPowerState != PowerEventState::LOW_BATTERY)
    {
        lastPowerState = PowerEventState::LOW_BATTERY;
        sendMessage("ALERT: POWER LOW | Voltage: " + std::to_string(currentVoltage) + " V");
        lastEventTime = currentTime;
    }
    else if (currentVoltage > VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::OVERCHARGE)
    {
        lastPowerState = PowerEventState::OVERCHARGE;
        sendMessage("ALERT: OVERVOLTAGE | Voltage: " + std::to_string(currentVoltage) + " V");
        lastEventTime = currentTime;
    }
    else if (currentVoltage >= VOLTAGE_LOW_THRESHOLD && currentVoltage <= VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::NORMAL)
    {
        lastPowerState = PowerEventState::NORMAL;
        sendMessage("INFO: Battery back to normal | Voltage: " + std::to_string(currentVoltage) + " V");
        lastEventTime = currentTime;
    }
}

/**
 * @brief Logs an event message to stdout (and optionally file).
 * @param message The event message.
 */
void logEvent(const char* message) {
    std::cout << message << std::endl;

    // Send the event message to Core 1 for logging
    std::string msg = message;
    int messageLength = msg.length();

    // Send the event log signal
    multicore_fifo_push_blocking(EVENT_LOG_SIGNAL);

    // Send the length of the message
    multicore_fifo_push_blocking(messageLength);

    // Send the message itself
    for (char c : msg) {
        multicore_fifo_push_blocking(c);
    }
}