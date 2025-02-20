#include "communication.h"
#include "commands.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

/**
 * @defgroup DiagnosticCommands Diagnostic Commands
 * @{
 */

/**
 * @brief Handler for listing all available commands on UART
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing success/error and command list
 * @note <b>KBST;0;GET;1;0;;TSBK</b>
 * @note Print all available commands on UART port
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 0
 */
Frame handle_get_commands_list(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 1, 0, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return frame_build(ExecutionResult::ERROR, 1, 0, "INVALID OPERATION");
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
    uart_print(commandList, VerbosityLevel::INFO); // Print to UART

    return frame_build(ExecutionResult::SUCCESS, 1, 0, "Commands listed on UART");
}

/**
 * @brief Get firmware build version
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing build number
 * @note <b>KBST;0;GET;1;1;;TSBK</b>
 * @note Get the firmware build version
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1
 */
Frame handle_get_build_version(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 1, 1, "PARAM UNECESSARY");
    }
    if (operationType == OperationType::GET) {
        return frame_build(ExecutionResult::SUCCESS, 1, 1, std::to_string(BUILD_NUMBER));
    }
    return frame_build(ExecutionResult::ERROR, 1, 1, "INVALID OPERATION");
}


/**
 * @brief Handles setting or getting the UART verbosity level.
 *
 * This function allows the user to either retrieve the current UART verbosity
 * level or set a new verbosity level.
 *
 * @param param The desired verbosity level (0-5) as a string.  If empty, the
 *              current level is returned.
 * @param operationType The operation type.  Must be GET to retrieve the current
 *                      level, or SET to set a new level.
 * @return A Frame indicating the result of the operation.
 *         - Success (GET): Frame containing the current verbosity level.
 *         - Success (SET): Frame with "LEVEL SET" message.
 *         - Error: Frame with error message (e.g., "INVALID LEVEL (0-5)", "INVALID FORMAT").
 *
 * @note <b>KBST;0;GET;1;8;;TSBK</b> - Gets the current verbosity level.
 * @note <b>KBST;0;SET;1;8;[level];TSBK</b> - Sets the verbosity level.
 * @note Example: <b>KBST;0;SET;1;8;2;TSBK</b> - Sets the verbosity level to 2.
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1.8
 */
Frame handle_verbosity(const std::string& param, OperationType operationType) {
    if (param.empty()) {
        uart_print("Current verbosity level: " + std::to_string(static_cast<int>(g_uart_verbosity)),  VerbosityLevel::INFO);
        return frame_build(ExecutionResult::SUCCESS, 1, 8, 
                         std::to_string(static_cast<int>(g_uart_verbosity)));
    }

    try {
        int level = std::stoi(param);
        if (level < 0 || level > 5) {
            return frame_build(ExecutionResult::ERROR, 1, 8, "INVALID LEVEL (0-5)");
        }
        g_uart_verbosity = static_cast<VerbosityLevel>(level);
        return frame_build(ExecutionResult::SUCCESS, 1, 8, "LEVEL SET");
    } catch (...) {
        return frame_build(ExecutionResult::ERROR, 1, 8, "INVALID FORMAT");
    }
}

/**
 * @brief Reboot system to USB firmware loader
 * @param param Empty string expected
 * @param operationType Must be SET
 * @return Frame with operation result
 * @note <b>KBST;0;SET;1;9;;TSBK</b>
 * @note Reboot the system to USB firmware loader
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2
 */
Frame handle_enter_bootloader_mode(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 1, 9, "PARAM UNNECESSARY");
    }

    if (operationType != OperationType::SET) {
        return frame_build(ExecutionResult::ERROR, 1, 9, "INVALID OPERATION");
    }

    // Build the success frame *before* resetting
    Frame successFrame = frame_build(ExecutionResult::SUCCESS, 1, 9, "REBOOT BOOTSEL");

    // Send the success frame
    uart_print("Sending BOOTSEL confirmation...");
    send_frame(successFrame); // Assuming you have a sendFrame function

    // Delay to ensure the frame is sent
    sleep_ms(100);

    uart_print("Entering BOOTSEL mode...", VerbosityLevel::WARNING);
    reset_usb_boot(0, 0); // Trigger BOOTSEL mode

    // The code will never reach here because the Pico will reset
    return frame_build(ExecutionResult::SUCCESS, 1, 9, "Entering BOOTSEL mode");
}

/** @} */ 