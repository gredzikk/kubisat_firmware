#include "event_manager.h"
#include <cstdio>
#include "protocol.h"
#include "pico/multicore.h"
#include "communication.h"
#include "utils.h"

volatile uint16_t eventLogId = 0;

static PowerEvent lastPowerState = PowerEvent::LOW_BATTERY;

static constexpr float FALL_RATE_THRESHOLD = -0.02f;
static constexpr int FALLING_TREND_REQUIRED = 3;
static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;

static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f;

static int fallingTrendCount = 0;

bool lastSolarState = false;
bool lastUSBState = false;

void logEvent(uint8_t group, uint8_t event) {
    EventLog log;
    log.id = eventLogId++;
    log.timestamp = to_ms_since_boot(get_absolute_time());
    log.group = group;
    log.event = event;

    uartPrint(log.toString());
}
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

    if (fallingTrendCount >= FALLING_TREND_REQUIRED) {
        uartPrint("Power falling detected!");
        lastPowerState = PowerEvent::POWER_FALLING;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::POWER_FALLING);
    }

    if (currentVoltage < PowerManager::VOLTAGE_LOW_THRESHOLD && 
        lastPowerState != PowerEvent::LOW_BATTERY) {
        uartPrint("Low battery detected!");
        lastPowerState = PowerEvent::LOW_BATTERY;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::LOW_BATTERY);
    } 
    else if (currentVoltage > PowerManager::VOLTAGE_OVERCHARGE_THRESHOLD && 
             lastPowerState != PowerEvent::OVERCHARGE) {
        uartPrint("Overcharge detected!");
        lastPowerState = PowerEvent::OVERCHARGE;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::OVERCHARGE);
    } 
    else if (currentVoltage >= PowerManager::VOLTAGE_LOW_THRESHOLD && 
             currentVoltage <= PowerManager::VOLTAGE_OVERCHARGE_THRESHOLD && 
             lastPowerState != PowerEvent::POWER_NORMAL) {
        uartPrint("Power back to normal!");
        lastPowerState = PowerEvent::POWER_NORMAL;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::POWER_NORMAL);
    }

    // Check solar charging state
    bool currentSolarState = pm.isSolarActive();
    if (currentSolarState != lastSolarState) {
        if (currentSolarState) {
            uartPrint("Solar charging active!");
            EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_ACTIVE);
        } else {
            uartPrint("Solar charging inactive!");
            EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_INACTIVE);
        }
        lastSolarState = currentSolarState;
    }

    // Check USB connection state
    bool currentUSBState = pm.isUSBConnected();
    if (currentUSBState != lastUSBState) {
        if (currentUSBState) {
            uartPrint("USB connected!");
            EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_CONNECTED);
        } else {
            uartPrint("USB disconnected!");
            EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_DISCONNECTED);
        }
        lastUSBState = currentUSBState;
    }
}