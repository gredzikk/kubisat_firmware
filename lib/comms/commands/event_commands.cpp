#include "communication.h"
#include "event_manager.h"
#include <sstream>


/**
 * @defgroup EventCommands Event Commands
 * @brief Commands for accessing and managing system event logs
 * @{
 */

/**
 * @brief Handler for retrieving last N events from the event log
 * @param param Number of events to retrieve (optional, default 10)
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Hex-encoded events in format IIIITTTTTTTTGGEE separated by '-'
 *           where:
 *           - IIII: Event ID (16-bit)
 *           - TTTTTTTT: Unix Timestamp (32-bit)
 *           - GG: Event Group (8-bit)
 *           - EE: Event Type (8-bit)
 *         - Error: "INVALID OPERATION", "INVALID COUNT", or "INVALID PARAMETER"
 * @note KBST;0;GET;5;1;20;TSBK
 *       Returns up to 20 most recent events
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.1
 */
Frame handle_get_last_events(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET) {
        return frame_build(ExecutionResult::ERROR, 5, 1, "INVALID OPERATION");
    }

    size_t count = 10; // Default number of events to return
    if (!param.empty()) {
        try {
            count = std::stoul(param);
            if (count == 0 || count > EVENT_BUFFER_SIZE) {
                return frame_build(ExecutionResult::ERROR, 5, 1, "INVALID COUNT");
            }
        } catch (...) {
            return frame_build(ExecutionResult::ERROR, 5, 1, "INVALID PARAMETER");
        }
    }

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    
    size_t available = eventManager.get_event_count();
    size_t toReturn = std::min(count, available);
    
    // Start from the most recent event
    for (size_t i = 0; i < toReturn; i++) {
        const EventLog& event = eventManager.get_event(available - 1 - i);
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

    return frame_build(ExecutionResult::SUCCESS, 5, 1, ss.str());
}


/**
 * @brief Handler for getting total number of events in the log
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Number of events currently in the log
 *         - Error: "INVALID REQUEST"
 * @note KBST;0;GET;5;2;;TSBK
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.2
 */
Frame hadnle_get_event_count(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET || !param.empty()) {
        return frame_build(ExecutionResult::ERROR, 5, 2, "INVALID REQUEST");
    }

    return frame_build(ExecutionResult::SUCCESS, 5, 2, 
                     std::to_string(eventManager.get_event_count()));
}
/** @} */ // end of EventCommands group