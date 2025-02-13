#include "communication.h"
#include <time.h> 

Frame handleTime(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting current time");
        time_t currentTime;
        time(&currentTime); // Get current time in seconds since epoch
        return buildSuccessFrame(1, 3, 0, std::to_string(currentTime), "");
    } else if (operationType == OperationType::SET) {
        uartPrint("Setting current time");
        try {
            time_t newTime = std::stoll(param); // Convert parameter to time_t
            time(&newTime);
            uartPrint("New time: " + std::to_string(newTime));
            return buildSuccessFrame(1, 3, 0, std::to_string(newTime) + " OK", "");
        } catch (const std::invalid_argument& e) {
            return buildErrorFrame(1, 3, 0, "INVALID PARAM");
        }
    } else {
        return buildErrorFrame(1, 3, 0, "INVALID OPERATION");
    }
}