#include "communication.h"

static constexpr uint8_t power_commands_group_id = 2;
static constexpr uint8_t powerman_ids_command_id = 0;
static constexpr uint8_t voltage_battery_command_id = 2;
static constexpr uint8_t voltage_main_command_id = 3;
static constexpr uint8_t charge_usb_command_id = 4;
static constexpr uint8_t charge_solar_command_id = 5;
static constexpr uint8_t charge_total_command_id = 6;
static constexpr uint8_t discharge_total_command_id = 7;

/**
 * @defgroup PowerCommands Power Commands
 * @brief Commands for monitoring power subsystem and battery management
 * @{
 */

/**
 * @brief Handler for retrieving Power Manager IDs
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: String of Power Manager IDs
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;0;;TSBK</b>
 * @note This command is used to retrieve the IDs of the Power Manager
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.0
 */
std::vector<Frame> handle_get_power_manager_ids(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, powerman_ids_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, powerman_ids_command_id, error_msg));
        return frames;
    }

    std::string power_manager_ids = PowerManager::get_instance().read_device_ids();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, powerman_ids_command_id, power_manager_ids));
    return frames;
}

/**
 * @brief Handler for getting battery voltage
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Battery voltage in Volts
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;2;;TSBK</b>
 * @note This command is used to retrieve the battery voltage
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.2
 */
std::vector<Frame> handle_get_voltage_battery(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, voltage_battery_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, voltage_battery_command_id, error_msg));
        return frames;
    }

    float voltage = PowerManager::get_instance().get_voltage_battery();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, voltage_battery_command_id, std::to_string(voltage), ValueUnit::VOLT));
    return frames;
}

/**
 * @brief Handler for getting 5V rail voltage
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: 5V rail voltage in Volts
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;3;;TSBK</b>
 * @note This command is used to retrieve the 5V rail voltage
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.3
 */
std::vector<Frame> handle_get_voltage_5v(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, voltage_main_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, voltage_main_command_id, error_msg));
        return frames;
    }

    float voltage = PowerManager::get_instance().get_voltage_5v();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, voltage_main_command_id, std::to_string(voltage), ValueUnit::VOLT));
    return frames;
}

/**
 * @brief Handler for getting USB charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: USB charge current in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;4;;TSBK</b>
 * @note This command is used to retrieve the USB charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.4
 */
std::vector<Frame> handle_get_current_charge_usb(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_usb_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_usb_command_id, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_usb();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, charge_usb_command_id, std::to_string(charge_current), ValueUnit::MILIAMP));
    return frames;
}

/**
 * @brief Handler for getting solar panel charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Solar charge current in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;5;;TSBK</b>
 * @note This command is used to retrieve the solar panel charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.5
 */
std::vector<Frame> handle_get_current_charge_solar(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_solar_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_solar_command_id, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_solar();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, charge_solar_command_id, std::to_string(charge_current), ValueUnit::MILIAMP));
    return frames;
}

/**
 * @brief Handler for getting total charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Total charge current (USB + Solar) in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;6;;TSBK</b>
 * @note This command is used to retrieve the total charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.6
 */
std::vector<Frame> handle_get_current_charge_total(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_total_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, charge_total_command_id, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_total();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, charge_total_command_id, std::to_string(charge_current), ValueUnit::MILIAMP));
    return frames;
}

/**
 * @brief Handler for getting system current draw
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: System current consumption in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;7;;TSBK</b>
 * @note This command is used to retrieve the system current draw
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.7
 */
std::vector<Frame> handle_get_current_draw(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (!param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, discharge_total_command_id, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, power_commands_group_id, discharge_total_command_id, error_msg));
        return frames;
    }

    float current_draw = PowerManager::get_instance().get_current_draw();
    frames.push_back(frame_build(OperationType::VAL, power_commands_group_id, discharge_total_command_id, std::to_string(current_draw), ValueUnit::MILIAMP));
    return frames;
}
/** @} */ // end of PowerCommands group