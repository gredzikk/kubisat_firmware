#include "event_manager.h"
#include <cstdio>
#include "protocol.h"
#include "pico/multicore.h"
#include "communication.h"
#include "utils.h"
#include "DS3231.h"

/**
 * @file event_manager.cpp
 * @brief Implements the event management system for the Kubisat firmware.
 * @details This file contains the implementation for logging events, managing event storage,
 *          and checking for specific events such as power status changes.
 * @ingroup EventManagerGroup
 */


 /**
 * @brief Global event log ID counter.
 * @ingroup EventManagerGroup
 */
volatile uint16_t eventLogId = 0;

/**
 * @brief Stores the last known power state.
 * @ingroup EventManagerGroup
 */
static PowerEvent lastPowerState = PowerEvent::LOW_BATTERY;

/**
 * @brief Threshold for detecting a falling voltage rate.
 * @ingroup EventManagerGroup
 */
static constexpr float FALL_RATE_THRESHOLD = -0.02f;

/**
 * @brief Number of consecutive falling voltage readings required to trigger a power falling event.
 * @ingroup EventManagerGroup
 */
static constexpr int FALLING_TREND_REQUIRED = 3;

/**
 * @brief Voltage threshold for detecting a low battery condition.
 * @ingroup EventManagerGroup
 */
static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;

/**
 * @brief Voltage threshold for detecting an overcharge condition.
 * @ingroup EventManagerGroup
 */
static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f;

/**
 * @brief Counter for consecutive falling voltage readings.
 * @ingroup EventManagerGroup
 */
static int fallingTrendCount = 0;

/**
 * @brief Stores the last known solar charging state.
 * @ingroup EventManagerGroup
 */
bool lastSolarState = false;

/**
 * @brief Stores the last known USB connection state.
 * @ingroup EventManagerGroup
 */
bool lastUSBState = false;

/**
 * @brief External declaration of the system clock.
 * @ingroup EventManagerGroup
 */
extern DS3231 systemClock;

/**
 * @brief Global instance of the EventManager implementation.
 * @ingroup EventManagerGroup
 */
EventManagerImpl eventManager;

uint16_t EventManager::nextEventId = 0;

/**
 * @brief Logs an event to the event buffer.
 * @param group The event group.
 * @param event The event ID.
 * @details Logs the event with a timestamp, group, and event ID. Prints the event to the UART,
 *          and saves the event to storage if the buffer is full or if it's a power-related event.
 * @ingroup EventManagerGroup
 */
void EventManager::log_event(uint8_t group, uint8_t event) {
    mutex_enter_blocking(&eventMutex);

    // Clear buffer if it's full
    if (eventCount >= EVENT_BUFFER_SIZE) {
        eventCount = 0;
        writeIndex = 0;
    }

    EventLog& log = events[writeIndex];
    log.id = nextEventId++;
    log.timestamp = systemClock.get_unix_time();
    log.group = group;
    log.event = event;

    // Print event immediately
    uart_print(log.to_string(), VerbosityLevel::EVENT);

    writeIndex = (writeIndex + 1) % EVENT_BUFFER_SIZE;
    eventCount++;
    eventsSinceFlush++;

    // Flush to storage every EVENT_FLUSH_THRESHOLD events
    if (eventsSinceFlush >= EVENT_FLUSH_THRESHOLD) {
        save_to_storage();
        eventsSinceFlush = 0;
    }

    mutex_exit(&eventMutex);
}


/**
 * @brief Retrieves an event from the event buffer.
 * @param index The index of the event to retrieve.
 * @return A const reference to the EventLog at the specified index. Returns an empty event if the index is out of bounds.
 * @ingroup EventManagerGroup
 */
const EventLog& EventManager::get_event(size_t index) const {
    static const EventLog emptyEvent = {0, 0, 0, 0};  // Initialize {id, timestamp, group, event}
    if (index >= eventCount) {
        return emptyEvent;
    }
    
    // Calculate actual index in circular buffer
    size_t actualIndex;
    if (eventCount == EVENT_BUFFER_SIZE) {
        actualIndex = (writeIndex + index) % EVENT_BUFFER_SIZE;
    } else {
        actualIndex = index;
    }
    
    return events[actualIndex];
}


/**
 * @brief Checks power statuses and triggers events based on voltage trends.
 * @param pm Reference to the PowerManager object.
 * @details Monitors the 5V voltage level, detects falling voltage trends, and triggers events
 *          for low battery, overcharge, and normal power conditions. Also checks solar charging
 *          and USB connection states.
 * @ingroup EventManagerGroup
 */
void check_power_events(PowerManager& pm) {
    float currentVoltage = pm.get_voltage_5v();
    static float previousVoltage = 0.0f;
    float delta = currentVoltage - previousVoltage;
    previousVoltage = currentVoltage;

    if (delta < FALL_RATE_THRESHOLD) {
        fallingTrendCount++;
    } else {
        fallingTrendCount = 0;
    }

    if (fallingTrendCount >= FALLING_TREND_REQUIRED) {
        lastPowerState = PowerEvent::POWER_FALLING;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::POWER_FALLING);
    }

    if (currentVoltage < PowerManager::VOLTAGE_LOW_THRESHOLD && 
        lastPowerState != PowerEvent::LOW_BATTERY) {
        lastPowerState = PowerEvent::LOW_BATTERY;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::LOW_BATTERY);
    } 
    else if (currentVoltage > PowerManager::VOLTAGE_OVERCHARGE_THRESHOLD && 
             lastPowerState != PowerEvent::OVERCHARGE) {
        lastPowerState = PowerEvent::OVERCHARGE;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::OVERCHARGE);
    } 
    else if (currentVoltage >= PowerManager::VOLTAGE_LOW_THRESHOLD && 
             currentVoltage <= PowerManager::VOLTAGE_OVERCHARGE_THRESHOLD && 
             lastPowerState != PowerEvent::POWER_NORMAL) {
        lastPowerState = PowerEvent::POWER_NORMAL;
        EventEmitter::emit(EventGroup::POWER, PowerEvent::POWER_NORMAL);
    }

    // Check solar charging state
    bool currentSolarState = pm.is_charging_solar();
    if (currentSolarState != lastSolarState) {
        if (currentSolarState) {
            EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_ACTIVE);
        } else {
            EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_INACTIVE);
        }
        lastSolarState = currentSolarState;
    }

    // Check USB connection state
    bool currentUSBState = pm.is_charging_usb();
    if (currentUSBState != lastUSBState) {
        if (currentUSBState) {
            EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_CONNECTED);
        } else {
            EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_DISCONNECTED);
        }
        lastUSBState = currentUSBState;
    }
}