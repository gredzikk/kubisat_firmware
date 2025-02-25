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
 * @param param Number of events to retrieve (optional, default 10). If 0, all events are returned.
 * @param operationType GET
 * @return Frame containing:
 *         - Success: A sequence of frames, each containing up to 10 hex-encoded events.
 *           Each event is in the format IIIITTTTTTTTGGEE, separated by '-'.
 *           - IIII: Event ID (16-bit, 4 hex characters)
 *           - TTTTTTTT: Unix Timestamp (32-bit, 8 hex characters)
 *           - GG: Event Group (8-bit, 2 hex characters)
 *           - EE: Event Type (8-bit, 2 hex characters)
 *           The last frame in the sequence is a VAL frame with the message "SEQ_DONE".
 *         - Error: A single frame with an error message:
 *           - "INVALID OPERATION": If the operation type is not GET.
 *           - "INVALID COUNT": If the count is greater than EVENT_BUFFER_SIZE.
 *           - "INVALID PARAMETER": If the parameter is not a valid unsigned integer.
 * @note <b>KBST;0;GET;5;1;[N];TSBK</b> - Retrieves the last N events. If N is 0, retrieves all events.
 * @note Returns up to 10 most recent events per frame.
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.1
 */
std::vector<Frame> handle_get_last_events(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET) {
        frames.push_back(frame_build(OperationType::ERR, 5, 1, "INVALID OPERATION"));
        return frames;
    }
    size_t count = 10; // Default number of events to return
    if (!param.empty()) {
        try {
            count = std::stoul(param);
            if (count > EVENT_BUFFER_SIZE) {
                frames.push_back(frame_build(OperationType::ERR, 5, 1, "INVALID COUNT"));
                return frames;
            }
        } catch (...) {
            frames.push_back(frame_build(OperationType::ERR, 5, 1, "INVALID PARAMETER"));
            return frames;
        }
    }

    size_t available = eventManager.get_event_count();
    size_t toReturn = (count == 0) ? available : std::min(count, available);
    size_t eventIndex = available;

    while (toReturn > 0) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        size_t eventsInFrame = 0;

        for (size_t i = 0; i < 10 && toReturn > 0; ++i) {
            eventIndex--;
            const EventLog& event = eventManager.get_event(eventIndex);

            ss << std::setw(4) << event.id
               << std::setw(8) << event.timestamp
               << std::setw(2) << static_cast<int>(event.group)
               << std::setw(2) << static_cast<int>(event.event);

            if (toReturn > 1) ss << "-";

            toReturn--;
            eventsInFrame++;
        }
        frames.push_back(frame_build(OperationType::SEQ, 5, 1, ss.str()));
    }
    frames.push_back(frame_build(OperationType::VAL, 5, 1, "SEQ_DONE"));
    return frames;
}


/**
 * @brief Handler for getting total number of events in the log
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Number of events currently in the log
 *         - Error: "INVALID REQUEST"
 * @note <b>KBST;0;GET;5;2;;TSBK</b>
 * @note Returns the total number of events in the log
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.2
 */
std::vector<Frame> handle_get_event_count(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET || !param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 5, 2, "INVALID REQUEST"));
        return frames;
    }
    frames.push_back(frame_build(OperationType::VAL, 5, 2, std::to_string(eventManager.get_event_count())));
    return frames;
}
/** @} */ // end of EventCommands group