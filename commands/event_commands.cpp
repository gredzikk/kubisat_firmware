#include "communication.h"
#include "event_manager.h"
#include <sstream>

Frame handleGetLastEvents(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET) {
        return buildFrame(ExecutionResult::ERROR, 5, 1, "INVALID OPERATION");
    }

    size_t count = 10; // Default number of events to return
    if (!param.empty()) {
        try {
            count = std::stoul(param);
            if (count == 0 || count > EVENT_BUFFER_SIZE) {
                return buildFrame(ExecutionResult::ERROR, 5, 1, "INVALID COUNT");
            }
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 5, 1, "INVALID PARAMETER");
        }
    }

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    
    size_t available = eventManager.getEventCount();
    size_t toReturn = std::min(count, available);
    
    // Start from the most recent event
    for (size_t i = 0; i < toReturn; i++) {
        const EventLog& event = eventManager.getEvent(available - 1 - i);
        // Format: IIIITTTTTTTTGGEE
        // IIII: 16-bit ID (4 hex chars)
        // TTTTTTTT: 32-bit timestamp (8 hex chars)
        // GG: 8-bit group (2 hex chars)
        // EE: 8-bit event (2 hex chars)
        ss << std::setw(4) << event.id
           << std::setw(8) << event.timestamp
           << std::setw(2) << static_cast<int>(event.group)
           << std::setw(2) << static_cast<int>(event.event);
        if (i < toReturn - 1) ss << "-";
    }

    return buildFrame(ExecutionResult::SUCCESS, 5, 1, ss.str());
}

Frame handleGetEventCount(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 5, 2, "INVALID REQUEST");
    }

    return buildFrame(ExecutionResult::SUCCESS, 5, 2, 
                     std::to_string(eventManager.getEventCount()));
}