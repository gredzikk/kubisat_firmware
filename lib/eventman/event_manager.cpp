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
 */


/**
 * @brief Global event log ID counter.
 */
volatile uint16_t eventLogId = 0;

/**
 * @brief Stores the last known power state.
 */
static PowerEvent lastPowerState = PowerEvent::LOW_BATTERY;

/**
 * @brief Threshold for detecting a falling voltage rate.
 */
static constexpr float FALL_RATE_THRESHOLD = -0.02f;

/**
 * @brief Number of consecutive falling voltage readings required to trigger a power falling event.
 */
static constexpr int FALLING_TREND_REQUIRED = 3;

/**
 * @brief Voltage threshold for detecting a low battery condition.
 */
static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;

/**
 * @brief Voltage threshold for detecting an overcharge condition.
 */
static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f;

/**
 * @brief Counter for consecutive falling voltage readings.
 */
static int fallingTrendCount = 0;

/**
 * @brief Stores the last known solar charging state.
 */
bool lastSolarState = false;

/**
 * @brief Stores the last known USB connection state.
 */
bool lastUSBState = false;

/**
 * @brief External declaration of the system clock.
 */
extern DS3231 systemClock;

/**
 * @brief Global instance of the EventManager implementation.
 */
EventManagerImpl eventManager;


/**
 * @brief Logs an event to the event buffer.
 * @param group The event group.
 * @param event The event ID.
 * @details Logs the event with a timestamp, group, and event ID. Prints the event to the UART,
 *          and saves the event to storage if the buffer is full or if it's a power-related event.
 */
void EventManager::logEvent(uint8_t group, uint8_t event) {
    mutex_enter_blocking(&eventMutex);

    EventLog& log = events[writeIndex];
    log.id = nextEventId++;
    log.timestamp = systemClock.getTimeUnix();
    log.group = group;
    log.event = event;

    // Print event immediately
    uartPrint(log.toString());

    writeIndex = (writeIndex + 1) % EVENT_BUFFER_SIZE;
    if (eventCount < EVENT_BUFFER_SIZE) {
        eventCount++;
    }

    // Set persistence flag on buffer full or power events
    if (eventCount == EVENT_BUFFER_SIZE || 
        (group == static_cast<uint8_t>(EventGroup::POWER) && 
         event == static_cast<uint8_t>(PowerEvent::POWER_FALLING))) {
        needsPersistence = true;
        saveToStorage();
    }

    mutex_exit(&eventMutex);
}


/**
 * @brief Retrieves an event from the event buffer.
 * @param index The index of the event to retrieve.
 * @return A const reference to the EventLog at the specified index. Returns an empty event if the index is out of bounds.
 */
const EventLog& EventManager::getEvent(size_t index) const {
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