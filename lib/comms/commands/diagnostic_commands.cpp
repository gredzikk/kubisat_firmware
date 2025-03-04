#include "communication.h"
#include "commands.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 
#include "system_state_manager.h"
/**
 * @defgroup DiagnosticCommands Diagnostic Commands
 * @{
 */

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
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, 1, 0, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, 1, 0, error_msg));
        return frames;
    }

    std::string combined_command_details;
    for (const auto& entry : command_handlers) {
        uint32_t command_key = entry.first;
        uint8_t group = (command_key >> 8) & 0xFF;
        uint8_t command = command_key & 0xFF;

        std::string command_details = std::to_string(group) + "." + std::to_string(command);

        if (combined_command_details.length() + command_details.length() + 1 > 100) {
            frames.push_back(frame_build(OperationType::SEQ, 1, 0, combined_command_details));
            combined_command_details = "";
        }

        if (!combined_command_details.empty()) {
            combined_command_details += "-";
        }
        combined_command_details += command_details;
    }

    if (!combined_command_details.empty()) {
        frames.push_back(frame_build(OperationType::SEQ, 1, 0, combined_command_details));
    }

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
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, 1, 1, error_msg));
        return frames;
    }
    
    if (operationType == OperationType::GET) {
        frames.push_back(frame_build(OperationType::VAL, 1, 1, std::to_string(BUILD_NUMBER)));
        return frames;
    }

    error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
    frames.push_back(frame_build(OperationType::ERR, 1, 1, error_msg));
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
    std::string error_msg;

    if (operationType == OperationType::GET && param.empty()) {
        VerbosityLevel current_level = SystemStateManager::get_instance().get_uart_verbosity();
        uart_print("GET_VERBOSITY_ " + std::to_string(static_cast<int>(current_level)), 
                  VerbosityLevel::INFO);
        frames.push_back(frame_build(OperationType::VAL, 1, 8, 
                        std::to_string(static_cast<int>(current_level))));
        return frames;
    }

    try {
        int level = std::stoi(param);
        if (level < 0 || level > 5) {
            error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
            frames.push_back(frame_build(OperationType::ERR, 1, 8, error_msg));
            return frames;
        }
        SystemStateManager::get_instance().set_uart_verbosity(static_cast<VerbosityLevel>(level));
        frames.push_back(frame_build(OperationType::RES, 1, 8, "LEVEL SET"));
        return frames;
    } catch (...) {
        error_msg = error_code_to_string(ErrorCode::INVALID_FORMAT);
        frames.push_back(frame_build(OperationType::ERR, 1, 8, error_msg));
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
    std::string error_msg;

    if (param != "USB") {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
        frames.push_back(frame_build(OperationType::ERR, 1, 9, error_msg));
        return frames;
    }

    if (operationType != OperationType::SET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, 1, 9, error_msg));
        return frames;
    }

    frames.push_back(frame_build(OperationType::RES, 1, 9, "REBOOT BOOTSEL"));
    
    SystemStateManager::get_instance().set_bootloader_reset_pending(true);
    
    return frames;
}

/** @} */ 