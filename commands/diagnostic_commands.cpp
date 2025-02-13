#include "communication.h"

Frame handleGetBuildVersion(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 1, 1, "PARAM UNECESSARY");
    }
    if (operationType == OperationType::GET) {
        return buildFrame(ExecutionResult::SUCCESS, 1, 1, std::to_string(BUILD_NUMBER));
    }
    return buildFrame(ExecutionResult::ERROR, 1, 1, "INVALID OPERATION");
}