#include "communication.h"

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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 0, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 0, "INVALID OPERATION"));
        return frames;
    }

    extern PowerManager powerManager;
    std::string powerManagerIDS = powerManager.read_device_ids();
    frames.push_back(frame_build(OperationType::VAL, 2, 0, powerManagerIDS));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 2, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 2, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float voltage = powerManager.get_voltage_battery();
    frames.push_back(frame_build(OperationType::VAL, 2, 2, std::to_string(voltage), ValueUnit::VOLT));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 3, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 3, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float voltage = powerManager.get_voltage_5v();
    frames.push_back(frame_build(OperationType::VAL, 2, 3, std::to_string(voltage), ValueUnit::VOLT));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 4, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 4, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_usb();
    frames.push_back(frame_build(OperationType::VAL, 2, 4, std::to_string(chargeCurrent), ValueUnit::MILIAMP));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 5, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 5, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_solar();
    frames.push_back(frame_build(OperationType::VAL, 2, 5, std::to_string(chargeCurrent), ValueUnit::MILIAMP));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 6, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 6, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_total();
    frames.push_back(frame_build(OperationType::VAL, 2, 6, std::to_string(chargeCurrent), ValueUnit::MILIAMP));
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
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 2, 7, "PARAM UNECESSARY"));
        return frames;
    }

    if (!(operationType == OperationType::GET)) {
        frames.push_back(frame_build(OperationType::ERR, 2, 7, "NOT ALLOWED"));
        return frames;
    }

    extern PowerManager powerManager;
    float currentDraw = powerManager.get_current_draw();
    frames.push_back(frame_build(OperationType::VAL, 2, 7, std::to_string(currentDraw), ValueUnit::MILIAMP));
    return frames;
}
/** @} */ // end of PowerCommands group