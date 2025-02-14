#include "communication.h"

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