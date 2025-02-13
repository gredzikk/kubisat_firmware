#include "communication.h"

Frame handleGetBuildVersion(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildErrorFrame(1, 1, 1, "PARAM UNECESSARY");
    }
    if (operationType == OperationType::GET) {
        return buildSuccessFrame(1, 1, 1, std::to_string(BUILD_NUMBER), "");
    }
    return buildErrorFrame(1, 1, 1, "INVALID OPERATION");
}