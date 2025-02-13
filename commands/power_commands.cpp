#include "communication.h"

Frame handleGetPowerManagerIDs(const std::string& param, OperationType operationType) {
    if (!param.empty()) {
        return buildErrorFrame(1, 2, 0, "PARAM UNECESSARY");
    }

    if (!(operationType == OperationType::GET)) {
        return buildErrorFrame(1, 2, 0, "INVALID OPERATION");
    }

    extern PowerManager powerManager;
    std::string powerManagerIDS = powerManager.readIDs();
    return buildSuccessFrame(1, 2, 0, powerManagerIDS, "");
}

Frame handleGetVoltageBattery(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting battery voltage");
        extern PowerManager powerManager;
        float voltage = powerManager.getVoltageBattery();
        return buildSuccessFrame(1, 2, 2, std::to_string(voltage), "V");
    } else {
        uartPrint("SET operation not allowed for GetVoltageBattery");
        return buildErrorFrame(1, 2, 2, "NOT ALLOWED");
    }
}

Frame handleGetVoltage5V(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting 5V voltage");
        extern PowerManager powerManager;
        float voltage = powerManager.getVoltage5V();
        return buildSuccessFrame(1, 2, 3, std::to_string(voltage), "V");
    } else {
        uartPrint("SET operation not allowed for GetVoltage5V");
        return buildErrorFrame(1, 2, 3, "NOT ALLOWED");
    }
}

Frame handleGetCurrentChargeUSB(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting USB charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeUSB();
        return buildSuccessFrame(1, 2, 4, std::to_string(chargeCurrent), "mA");
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeUSB");
        return buildErrorFrame(1, 2, 4, "NOT ALLOWED");
    }
}

Frame handleGetCurrentChargeSolar(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting solar charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeSolar();
        return buildSuccessFrame(1, 2, 5, std::to_string(chargeCurrent), "mA");
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeSolar");
        return buildErrorFrame(1, 2, 5, "NOT ALLOWED");
    }
}

Frame handleGetCurrentChargeTotal(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting total charge current");
        extern PowerManager powerManager;
        float chargeCurrent = powerManager.getCurrentChargeTotal();
        return buildSuccessFrame(1, 2, 6, std::to_string(chargeCurrent), "mA");
    } else {
        uartPrint("SET operation not allowed for GetCurrentChargeTotal");
        return buildErrorFrame(1, 2, 6, "NOT ALLOWED");
    }
}

Frame handleGetCurrentDraw(const std::string& param, OperationType operationType) {
    if (operationType == OperationType::GET) {
        uartPrint("Getting current draw");
        extern PowerManager powerManager;
        float currentDraw = powerManager.getCurrentDraw();
        return buildSuccessFrame(1, 2, 7, std::to_string(currentDraw), "mA");
    } else {
        uartPrint("SET operation not allowed for GetCurrentDraw");
        return buildErrorFrame(1, 2, 7, "NOT ALLOWED");
    }
}