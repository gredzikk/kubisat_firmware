#include "communication.h"
#include "commands.h"

Frame handleGetBuildVersion(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 1, 1, "PARAM UNECESSARY");
    }
    if (operationType == OperationType::GET) {
        return buildFrame(ExecutionResult::SUCCESS, 1, 1, std::to_string(BUILD_NUMBER));
    }
    return buildFrame(ExecutionResult::ERROR, 1, 1, "INVALID OPERATION");
}

Frame handleListCommands(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 0, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 0, 0, "INVALID OPERATION");
    }

    std::stringstream ss;
    for (const auto& entry : commandHandlers) {
        uint32_t commandKey = entry.first;
        uint8_t group = (commandKey >> 8) & 0xFF;
        uint8_t command = commandKey & 0xFF;

        ss << "Group: " << static_cast<int>(group)
           << ", Command: " << static_cast<int>(command) << "\n";
    }

    std::string commandList = ss.str();
    uartPrint(commandList, true); // Print to UART

    return buildFrame(ExecutionResult::SUCCESS, 0, 0, "Commands listed on UART");
}