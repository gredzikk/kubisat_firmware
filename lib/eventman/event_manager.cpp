#include "event_manager.h"
#include <cstdio>
#include "protocol.h"
#include "pico/multicore.h"
#include "communication.h"
#include "utils.h"
#include "DS3231.h"

bool EventManager::init() {
    return load_from_storage();
}

void EventManager::log_event(uint8_t group, uint8_t event) {
    mutex_enter_blocking(&eventMutex);

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint16_t id = nextEventId++;

    EventLog& log = events[writeIndex];
    log.id = id;
    log.timestamp = timestamp;
    log.group = group;
    log.event = event;

    writeIndex = (writeIndex + 1) % EVENT_BUFFER_SIZE;
    if (eventCount < EVENT_BUFFER_SIZE) {
        eventCount++;
    }

    eventsSinceFlush++;

    mutex_exit(&eventMutex);

    std::string event_string = "Event: " + std::to_string(id) +
        " Group: " + std::to_string(group) +
        " Event: " + std::to_string(event);
    uart_print(event_string, VerbosityLevel::DEBUG);

    if (eventsSinceFlush >= EVENT_FLUSH_THRESHOLD || group == static_cast<uint8_t>(EventGroup::POWER)) {
        save_to_storage();
        eventsSinceFlush = 0;
    }
}

const EventLog& EventManager::get_event(size_t index) const {
    mutex_enter_blocking(const_cast<mutex_t*>(&eventMutex));
    if (index >= eventCount) {
        static EventLog emptyEvent;
        mutex_exit(const_cast<mutex_t*>(&eventMutex));
        return emptyEvent;
    }

    size_t readIndex;
    if (eventCount == EVENT_BUFFER_SIZE) {
        readIndex = (writeIndex + index) % EVENT_BUFFER_SIZE;
    }
    else {
        readIndex = index;
    }
    const EventLog& event = events[readIndex];
    mutex_exit(const_cast<mutex_t*>(&eventMutex));
    return event;
}

bool EventManager::save_to_storage() {
    if (!SystemStateManager::get_instance().is_sd_card_mounted()) {
        bool status = fs_init();
        if (!status) {
            return false;
        }
    }

    FILE* file = fopen(EVENT_LOG_FILE, "a");
    if (file) {
        size_t startIdx = (writeIndex >= eventsSinceFlush) ?
            writeIndex - eventsSinceFlush :
            EVENT_BUFFER_SIZE - (eventsSinceFlush - writeIndex);

        for (size_t i = 0; i < eventsSinceFlush; i++) {
            size_t idx = (startIdx + i) % EVENT_BUFFER_SIZE;
            fprintf(file, "%u;%lu;%u;%u\n",
                events[idx].id,
                events[idx].timestamp,
                events[idx].group,
                events[idx].event
            );
        }
        fclose(file);
        uart_print("Events saved to storage", VerbosityLevel::INFO);
        return true;
    }
    return false;
}