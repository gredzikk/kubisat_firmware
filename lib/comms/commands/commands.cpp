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
using CommandHandler = std::function<std::vector<Frame>(const std::string&, OperationType)>;

/**
 * @typedef CommandMap
 * @brief Map type for storing command handlers
 */
using CommandMap = std::map<uint32_t, CommandHandler>;

/**
 * @brief Global map of all command handlers
 * @details Maps command keys (group << 8 | command) to their handler functions
 */
CommandMap command_handlers = {
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(0)), handle_get_commands_list},               // Group 1, Command 0
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(1)), handle_get_build_version},               // Group 1, Command 1
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(8)), handle_verbosity},                       // Group 1, Command 9
    {((static_cast<uint32_t>(1) << 8) | static_cast<uint32_t>(9)), handle_enter_bootloader_mode},           // Group 1, Command 9
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(0)), handle_get_power_manager_ids},           // Group 2, Command 0
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(2)), handle_get_voltage_battery},             // Group 2, Command 2
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(3)), handle_get_voltage_5v},                  // Group 2, Command 3
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(4)), handle_get_current_charge_usb},          // Group 2, Command 4
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(5)), handle_get_current_charge_solar},        // Group 2, Command 5
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(6)), handle_get_current_charge_total},        // Group 2, Command 6
    {((static_cast<uint32_t>(2) << 8) | static_cast<uint32_t>(7)), handle_get_current_draw},                // Group 2, Command 7
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(0)), handle_time},                            // Group 3, Command 0
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(1)), handle_timezone_offset},                 // Group 3, Command 1
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(2)), handle_clock_sync_interval},             // Group 3, Command 2         
    {((static_cast<uint32_t>(3) << 8) | static_cast<uint32_t>(3)), handle_get_last_sync_time},              // Group 3, Command 3
    {((static_cast<uint32_t>(4) << 8) | static_cast<uint32_t>(0)), handle_get_sensor_data},                 // Group 4, Command 0
    {((static_cast<uint32_t>(4) << 8) | static_cast<uint32_t>(1)), handle_sensor_config},                   // Group 4, Command 1
    {((static_cast<uint32_t>(4) << 8) | static_cast<uint32_t>(3)), handle_get_sensor_list},                 // Group 4, Command 3
    {((static_cast<uint32_t>(5) << 8) | static_cast<uint32_t>(1)), handle_get_last_events},                 // Group 5, Command 1
    {((static_cast<uint32_t>(5) << 8) | static_cast<uint32_t>(2)), handle_get_event_count},                 // Group 5, Command 2
    {((static_cast<uint32_t>(6) << 8) | static_cast<uint32_t>(0)), handle_list_files},                      // Group 6, Command 0
    {((static_cast<uint32_t>(6) << 8) | static_cast<uint32_t>(4)), handle_mount},                           // Group 6, Command 4
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(1)), handle_gps_power_status},                // Group 7, Command 1
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(2)), handle_enable_gps_uart_passthrough},     // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(3)), handle_get_rmc_data},                    // Group 7, Command 3
    {((static_cast<uint32_t>(7) << 8) | static_cast<uint32_t>(4)), handle_get_gga_data},                    // Group 7, Command 4
    {((static_cast<uint32_t>(8) << 8) | static_cast<uint32_t>(2)), handle_get_last_telemetry_record},
    {((static_cast<uint32_t>(8) << 8) | static_cast<uint32_t>(3)), handle_get_last_sensor_record},
};


/**
 * @brief Executes a command based on its key
 * @param commandKey Combined group and command ID (group << 8 | command)
 * @param param Command parameter string
 * @param operationType Operation type (GET/SET)
 * @return Frame Response frame containing execution result
 * @details Looks up the command handler in commandHandlers map and executes it
 */
std::vector<Frame> execute_command(uint32_t commandKey, const std::string& param, OperationType operationType) {
    auto it = command_handlers.find(commandKey);
    if (it != command_handlers.end()) {
        CommandHandler handler = it->second;
        return handler(param, operationType);
    } else {
        std::vector<Frame> frames;
        frames.push_back(frame_build(OperationType::ERR, 0, 0, "INVALID COMMAND"));
        return frames;
    }
}
/** @} */ // end of CommandSystem group