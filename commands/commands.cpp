// commands/commands.cpp
#include "commands.h"
#include "communication.h"

/**
 * @defgroup CommandSystem Command System
 * @brief Core command system implementation
 * @{
 */

/**
 * @typedef CommandHandler
 * @brief Function type for command handlers
 */
using CommandHandler = std::function<Frame(const std::string&, OperationType)>;

/**
 * @typedef CommandMap
 * @brief Map type for storing command handlers
 */
using CommandMap = std::map<uint32_t, CommandHandler>;

/**
 * @brief Global map of all command handlers
 * @details Maps command keys (group << 8 | command) to their handler functions
 */
CommandMap commandHandlers = {
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(0)), handleListCommands},             // Group 1, Command 0
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(1)), handleGetBuildVersion},          // Group 1, Command 1
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(9)), handleEnterBootloaderMode},      // Group 9, Command 0
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(0)), handleGetPowerManagerIDs},       // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)), handleGetVoltageBattery},        // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(3)), handleGetVoltage5V},             // Group 2, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(4)), handleGetCurrentChargeUSB},      // Group 2, Command 4
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(5)), handleGetCurrentChargeSolar},    // Group 2, Command 5
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(6)), handleGetCurrentChargeTotal},    // Group 2, Command 6
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(7)), handleGetCurrentDraw},           // Group 2, Command 7
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)), handleTime},                     // Group 3, Command 0
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(1)), handleTimezoneOffset}, // Group 3, Command 1
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(2)), handleClockSyncInterval}, // Group 3, Command 3         // Group 3, Command 6
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(3)), handleGetLastSyncTime},          // Group 3, Command 7
    {((static_cast<uint32_t>(5) << 8) | static_cast<uint32_t>(1)), handleGetLastEvents},    // Group 5, Command 1
    {((static_cast<uint32_t>(5) << 8) | static_cast<uint32_t>(2)), handleGetEventCount},    // Group 5, Command 2
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(1)), handleGPSPowerStatus},           // Group 7, Command 1
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(2)), handleEnableGPSTransparentMode}, // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(3)), handleGetRMCData},               // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(4)), handleGetGGAData},               // Group 7, Command 4
};


/**
 * @brief Executes a command based on its key
 * @param commandKey Combined group and command ID (group << 8 | command)
 * @param param Command parameter string
 * @param operationType Operation type (GET/SET)
 * @return Frame Response frame containing execution result
 * @details Looks up the command handler in commandHandlers map and executes it
 */
Frame executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType) {
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        CommandHandler handler = it->second;
        return handler(param, operationType);
    } else {
        return buildFrame(ExecutionResult::ERROR, 0, 0, "INVALID COMMAND");
    }
}
/** @} */ // end of CommandSystem group