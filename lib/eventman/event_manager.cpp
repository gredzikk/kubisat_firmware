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
    uart_print(log.to_string(), VerbosityLevel::WARNING);

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
