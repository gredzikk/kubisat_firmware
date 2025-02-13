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

Frame handleListCommands(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 1, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 1, 0, "INVALID OPERATION");
    }

    std::string commandList;
    std::vector<Group> groups = getGroups();
    
    for (const auto& group : groups) {
        for (const auto& cmd : group.Commands) {
            std::string accessStr;
            switch (cmd.AccessRights) {
                case CommandAccessLevel::READ_ONLY: accessStr = "RO"; break;
                case CommandAccessLevel::READ_WRITE: accessStr = "RW"; break;
                case CommandAccessLevel::WRITE_ONLY: accessStr = "WO"; break;
                default: accessStr = "NA"; break;
            }
            
            // Format: GROUP_ID:CMD_ID - GROUP_NAME:CMD_NAME [ACCESS]
            std::string cmdInfo = std::to_string(group.Id) + ":" + 
                                std::to_string(cmd.Id) + " - " + 
                                group.Name + ":" + cmd.Name + 
                                " [" + accessStr + "]";
            uartPrint(cmdInfo, true);
        }
    }

    return buildFrame(ExecutionResult::SUCCESS, 1, 0, "Commands listed on UART");
}

Frame handleGetCommandsTimestamp(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 1, 3, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 1, 3, "INVALID OPERATION");
    }

    extern const uint32_t COMMANDS_FILE_VERSION;
    return buildFrame(ExecutionResult::SUCCESS, 1, 3, std::to_string(COMMANDS_FILE_VERSION));
}