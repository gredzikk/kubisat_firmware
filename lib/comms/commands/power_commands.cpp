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
Frame handleGetPowerManagerIDs(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 0, "PARAM UNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildFrame(ExecutionResult::ERROR, 2, 0, "INVALID OPERATION");
    }

    extern PowerManager powerManager;
    std::string powerManagerIDS = powerManager.readIDs();
    return buildFrame(ExecutionResult::SUCCESS, 2, 0, powerManagerIDS);
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
Frame handleGetVoltageBattery(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 2, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetVoltageBattery");
        return buildFrame(ExecutionResult::ERROR, 2, 2, "NOT ALLOWED");
    }

    uartPrint("Getting battery voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltageBattery();
    return buildFrame(ExecutionResult::SUCCESS, 2, 2, std::to_string(voltage), ValueUnit::VOLT);
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
Frame handleGetVoltage5V(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 3, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetVoltage5V");
        return buildFrame(ExecutionResult::ERROR, 2, 3, "NOT ALLOWED");
    }

    uartPrint("Getting 5V voltage");
    extern PowerManager powerManager;
    float voltage = powerManager.getVoltage5V();
    return buildFrame(ExecutionResult::SUCCESS, 2, 3, std::to_string(voltage), ValueUnit::VOLT);
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
Frame handleGetCurrentChargeUSB(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 4, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetCurrentChargeUSB");
        return buildFrame(ExecutionResult::ERROR, 2, 4, "NOT ALLOWED");
    }

    uartPrint("Getting USB charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeUSB();
    return buildFrame(ExecutionResult::SUCCESS, 2, 4, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
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
Frame handleGetCurrentChargeSolar(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 5, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetCurrentChargeSolar");
        return buildFrame(ExecutionResult::ERROR, 2, 5, "NOT ALLOWED");
    }

    uartPrint("Getting solar charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeSolar();
    return buildFrame(ExecutionResult::SUCCESS, 2, 5, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
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
Frame handleGetCurrentChargeTotal(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 6, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetCurrentChargeTotal");
        return buildFrame(ExecutionResult::ERROR, 2, 6, "NOT ALLOWED");
    }

    uartPrint("Getting total charge current");
    extern PowerManager powerManager;
    float chargeCurrent = powerManager.getCurrentChargeTotal();
    return buildFrame(ExecutionResult::SUCCESS, 2, 6, std::to_string(chargeCurrent), ValueUnit::MILIAMP);
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
Frame handleGetCurrentDraw(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildFrame(ExecutionResult::ERROR, 2, 7, "PARAM UNNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        uartPrint("SET operation not allowed for GetCurrentDraw");
        return buildFrame(ExecutionResult::ERROR, 2, 7, "NOT ALLOWED");
    }

    uartPrint("Getting current draw");
    extern PowerManager powerManager;
    float currentDraw = powerManager.getCurrentDraw();
    return buildFrame(ExecutionResult::SUCCESS, 2, 7, std::to_string(currentDraw), ValueUnit::MILIAMP);
}
/** @} */ // end of PowerCommands group