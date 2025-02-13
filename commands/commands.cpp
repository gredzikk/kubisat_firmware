// commands/commands.cpp
#include "commands.h"
#include "communication.h"

// Command handler function type
using CommandHandler = std::function<Frame(const std::string&, OperationType)>;
using CommandMap = std::map<uint32_t, CommandHandler>;

CommandMap commandHandlers = {
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(0)), handleListCommands},      // Group 1, Command 0
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(1)), handleGetBuildVersion},          // Group 1, Command 1
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(3)), handleGetCommandsTimestamp},     // Group 1, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(0)), handleGetPowerManagerIDs},        // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)), handleGetVoltageBattery},        // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(3)), handleGetVoltage5V},             // Group 2, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(4)), handleGetCurrentChargeUSB},      // Group 2, Command 4
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(5)), handleGetCurrentChargeSolar},    // Group 2, Command 5
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(6)), handleGetCurrentChargeTotal},    // Group 2, Command 6
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(7)), handleGetCurrentDraw},           // Group 2, Command 7
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)), handleTime},                     // Group 3, Command 0
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(1)), handleGPSPowerStatus},           // Group 7, Command 1
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(2)), handleEnableGPSTransparentMode}, // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(3)), handleGetGPSData},               // Group 7, Command 4
};

Frame executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType) {
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        CommandHandler handler = it->second;
        return handler(param, operationType);
    } else {
        return buildFrame(ExecutionResult::ERROR, 0, 0, "INVALID COMMAND");// Generic error
    }
}