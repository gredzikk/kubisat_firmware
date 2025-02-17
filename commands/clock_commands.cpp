#include "communication.h"
#include <time.h> 

Frame handleTime(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::SET && param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "PARAM REQUIRED");
    }

    if (operationType == OperationType::GET && !param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        return buildFrame(ExecutionResult::ERROR, 3, 0, "INVALID OPERATION");
    }

    if (operationType == OperationType::SET) {
        try {
            time_t newTime = std::stoll(param);
            if (newTime != 0) {
                return buildFrame(ExecutionResult::ERROR, 3, 0, "FAILED TO SET TIME");
            }
            EventEmitter::emit(EventGroup::CLOCK ,ClockEvent::CHANGED);
            return buildFrame(ExecutionResult::SUCCESS, 3, 0, "Time set successfully");
        } catch (...) {
            return buildFrame(ExecutionResult::ERROR, 3, 0, "INVALID TIME FORMAT");
        }
    }

    // GET operation
    time_t currentTime;
    time(&currentTime);
    return buildFrame(ExecutionResult::SUCCESS, 3, 0, std::to_string(currentTime));
}