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
 * @return Frame containing:
 *         - Success: String of Power Manager IDs
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;0;;TSBK</b>
 * @note This command is used to retrieve the IDs of the Power Manager
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.0
 */
Frame handle_get_power_manager_ids(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 0, "PARAM UNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return frame_build(ExecutionResult::ERROR, 2, 0, "INVALID OPERATION");
    }

    extern PowerManager powerManager;
    std::string powerManagerIDS = powerManager.read_device_ids();
    return frame_build(ExecutionResult::SUCCESS, 2, 0, powerManagerIDS);
}


/**
 * @brief Handler for getting battery voltage
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Battery voltage in Volts
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;2;;TSBK</b>
 * @note This command is used to retrieve the battery voltage
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.2
 */
Frame handle_get_voltage_battery(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 2, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetVoltageBattery");
        return frame_build(ExecutionResult::ERROR, 2, 2, "NOT ALLOWED");
    }

    uart_print("Getting battery voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.get_voltage_battery();
    return frame_build(ExecutionResult::SUCCESS, 2, 2, std::to_string(voltage), ValueUnit::VOLT);
}


/**
 * @brief Handler for getting 5V rail voltage
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: 5V rail voltage in Volts
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;3;;TSBK</b>
 * @note This command is used to retrieve the 5V rail voltage
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.3
 */
Frame handle_get_voltage_5v(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 3, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetVoltage5V");
        return frame_build(ExecutionResult::ERROR, 2, 3, "NOT ALLOWED");
    }

    uart_print("Getting 5V voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.get_voltage_5v();
    return frame_build(ExecutionResult::SUCCESS, 2, 3, std::to_string(voltage), ValueUnit::VOLT);
}


/**
 * @brief Handler for getting USB charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: USB charge current in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;4;;TSBK</b>
 * @note This command is used to retrieve the USB charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.4
 */
Frame handle_get_current_charge_usb(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 4, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetCurrentChargeUSB");
        return frame_build(ExecutionResult::ERROR, 2, 4, "NOT ALLOWED");
    }

    uart_print("Getting USB charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_usb();
    return frame_build(ExecutionResult::SUCCESS, 2, 4, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
}


/**
 * @brief Handler for getting solar panel charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Solar charge current in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;5;;TSBK</b>
 * @note This command is used to retrieve the solar panel charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.5
 */
Frame handle_get_current_charge_solar(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 5, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetCurrentChargeSolar");
        return frame_build(ExecutionResult::ERROR, 2, 5, "NOT ALLOWED");
    }

    uart_print("Getting solar charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_solar();
    return frame_build(ExecutionResult::SUCCESS, 2, 5, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
}


/**
 * @brief Handler for getting total charge current
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Total charge current (USB + Solar) in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;6;;TSBK</b>
 * @note This command is used to retrieve the total charge current
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.6
 */
Frame handle_get_current_charge_total(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 6, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetCurrentChargeTotal");
        return frame_build(ExecutionResult::ERROR, 2, 6, "NOT ALLOWED");
    }

    uart_print("Getting total charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.get_current_charge_total();
    return frame_build(ExecutionResult::SUCCESS, 2, 6, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
}


/**
 * @brief Handler for getting system current draw
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: System current consumption in milliamps
 *         - Error: Error message
 * @note <b>KBST;0;GET;2;7;;TSBK</b>
 * @note This command is used to retrieve the system current draw
 * @ingroup PowerCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2.7
 */
Frame handle_get_current_draw(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return frame_build(ExecutionResult::ERROR, 2, 7, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uart_print("SET operation not allowed for GetCurrentDraw");
        return frame_build(ExecutionResult::ERROR, 2, 7, "NOT ALLOWED");
    }

    
    extern PowerManager powerManager;
    float currentDraw = powerManager.get_current_draw();
    uart_print("Getting current draw");
    return frame_build(ExecutionResult::SUCCESS, 2, 7, std::to_string(currentDraw), ValueUnit::MILIAMP);
}
/** @} */ // end of PowerCommands group