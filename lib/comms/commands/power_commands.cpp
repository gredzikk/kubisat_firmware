#include "communication.h"

#define POWER_GROUP 2
#define POWER_MANAGER_IDS 0
#define VOLTAGE_BATTERY 2
#define VOLTAGE_MAIN 3
#define CHARGE_USB 4
#define CHARGE_SOLAR 5
#define CHARGE_TOTAL 6
#define DRAW_TOTAL 7

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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, POWER_MANAGER_IDS, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, POWER_MANAGER_IDS, error_msg));
        return frames;
    }

    std::string power_manager_ids = PowerManager::get_instance().read_device_ids();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, POWER_MANAGER_IDS, power_manager_ids));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, VOLTAGE_BATTERY, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, VOLTAGE_BATTERY, error_msg));
        return frames;
    }

    float voltage = PowerManager::get_instance().get_voltage_battery();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, VOLTAGE_BATTERY, std::to_string(voltage), ValueUnit::VOLT));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, VOLTAGE_MAIN, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, VOLTAGE_MAIN, error_msg));
        return frames;
    }

    float voltage = PowerManager::get_instance().get_voltage_5v();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, VOLTAGE_MAIN, std::to_string(voltage), ValueUnit::VOLT));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_USB, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_USB, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_usb();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, CHARGE_USB, std::to_string(charge_current), ValueUnit::MILIAMP));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_SOLAR, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_SOLAR, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_solar();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, CHARGE_SOLAR, std::to_string(charge_current), ValueUnit::MILIAMP));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_TOTAL, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, CHARGE_TOTAL, error_msg));
        return frames;
    }

    float charge_current = PowerManager::get_instance().get_current_charge_total();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, CHARGE_TOTAL, std::to_string(charge_current), ValueUnit::MILIAMP));
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
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, DRAW_TOTAL, error_msg));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, POWER_GROUP, DRAW_TOTAL, error_msg));
        return frames;
    }

    float current_draw = PowerManager::get_instance().get_current_draw();
    frames.push_back(frame_build(OperationType::VAL, POWER_GROUP, DRAW_TOTAL, std::to_string(current_draw), ValueUnit::MILIAMP));
    return frames;
}
/** @} */ // end of PowerCommands group