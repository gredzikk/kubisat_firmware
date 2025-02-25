#include "communication.h"
#include "commands.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

/**
 * @defgroup DiagnosticCommands Diagnostic Commands
 * @{
 */

extern volatile bool g_pending_bootloader_reset;

/**
 * @brief Handler for listing all available commands on UART
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of response frames - start frame, sequence of elements, end frame
 * @note <b>KBST;0;GET;1;0;;TSBK</b>
 * @note Print all available commands on UART port
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 0
 */
std::vector<Frame> handle_get_commands_list(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 1, 0, "PARAM_UNNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 1, 0, "INVALID_OPERATION"));
        return frames;
    }

    for (const auto& entry : commandHandlers) {
        uint32_t commandKey = entry.first;
        uint8_t group = (commandKey >> 8) & 0xFF;
        uint8_t command = commandKey & 0xFF;

        std::string commandDetails = std::to_string(group) + "-" + std::to_string(command);
        frames.push_back(frame_build(OperationType::SEQ, 1, 0, commandDetails));
    }

    // Add final VAL frame
    frames.push_back(frame_build(OperationType::VAL, 1, 0, "SEQ_DONE"));
    return frames;
}


/**
 * @brief Get firmware build version
 * @param param Empty string expected
 * @param operationType GET
 * @return One-element vector with result frame
 * @note <b>KBST;0;GET;1;1;;TSBK</b>
 * @note Get the firmware build version
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1
 */
std::vector<Frame> handle_get_build_version(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 1, 1, "PARAM_UNNECESSARY"));
        return frames;
    }
    
    if (operationType == OperationType::GET) {
        frames.push_back(frame_build(OperationType::VAL, 1, 1, std::to_string(BUILD_NUMBER)));
        return frames;
    }
    
    frames.push_back(frame_build(OperationType::ERR, 1, 1, "INVALID_OPERATION"));
    return frames;
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
 * @return Vector containing one frame indicating the result of the operation.
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
std::vector<Frame> handle_verbosity(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    
    if (operationType == OperationType::GET && param.empty()) {
        uart_print("Current verbosity level: " + std::to_string(static_cast<int>(g_uart_verbosity)), 
                  VerbosityLevel::INFO);
        frames.push_back(frame_build(OperationType::VAL, 1, 8, 
                        std::to_string(static_cast<int>(g_uart_verbosity))));
        return frames;
    }

    try {
        int level = std::stoi(param);
        if (level < 0 || level > 5) {
            frames.push_back(frame_build(OperationType::ERR, 1, 8, "INVALID LEVEL (0-5)"));
            return frames;
        }
        g_uart_verbosity = static_cast<VerbosityLevel>(level);
        frames.push_back(frame_build(OperationType::RES, 1, 8, "SET " + std::to_string(level)));
        return frames;
    } catch (...) {
        frames.push_back(frame_build(OperationType::ERR, 1, 8, "INVALID FORMAT"));
        return frames;
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
std::vector<Frame> handle_enter_bootloader_mode(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 1, 9, "PARAM_UNNECESSARY"));
        return frames;
    }

    if (operationType != OperationType::SET) {
        frames.push_back(frame_build(OperationType::ERR, 1, 9, "INVALID_OPERATION"));
        return frames;
    }

    frames.push_back(frame_build(OperationType::RES, 1, 9, "REBOOT BOOTSEL"));
    
    // Set flag to trigger reboot after function returns
    g_pending_bootloader_reset = true;
    
    return frames;
}

/** @} */ 