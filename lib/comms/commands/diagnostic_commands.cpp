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
    uart_print(commandList, true); // Print to UART

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

    uart_print("Entering BOOTSEL mode...");
    reset_usb_boot(0, 0); // Trigger BOOTSEL mode

    // The code will never reach here because the Pico will reset
    return frame_build(ExecutionResult::SUCCESS, 1, 9, "Entering BOOTSEL mode");
}

/** @} */ 