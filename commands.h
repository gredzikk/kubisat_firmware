#pragma once

#include <string>
#include <map>
#include "pico/stdlib.h"

enum class Command {
    GET_TIME,
    GET_VOLTAGE_BATTERY,
    GET_VOLTAGE_5V,
    GET_CURRENT_CHARGE_USB,
    GET_CURRENT_CHARGE_SOLAR,
    GET_CURRENT_CHARGE_TOTAL,
    GET_CURRENT_DRAW,
    GET_GPS_POWER_STATUS,
    SET_GPS_POWER_STATUS, 
    COMMANDS,
    UNKNOWN
};

void handleGetTime();
void handleGetVoltage5V();
void handleGetCurrentChargeUSB();
void handleGetCurrentChargeSolar();
void handleGetCurrentChargeTotal();
void handleGetCurrentDraw();
void handleGetGPSPowerStatus();
void handleSetGPSPowerStatus(const std::string& param);
void handleCommands();
void handleUnknownCommand();
void handleCommand(const std::string& message);
void sendMessage(std::string outgoing);

extern std::map<std::string, Command> commandMap;