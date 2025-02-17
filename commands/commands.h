// commands/commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <functional>
#include <map>
#include "protocol.h"

// CLOCK
Frame handleTime(const std::string& param, OperationType operationType);

// DIAG
Frame handleListCommands(const std::string& param, OperationType operationType);
Frame handleGetBuildVersion(const std::string& param, OperationType operationType);
Frame handleGetCommandsTimestamp(const std::string& param, OperationType operationType);

// GPS
Frame handleGPSPowerStatus(const std::string& param, OperationType operationType);
Frame handleEnableGPSTransparentMode(const std::string& param, OperationType operationType);
Frame handleGetGPSData(const std::string& param, OperationType operationType);

Frame handleGetGPSTime(const std::string& param, OperationType operationType);
Frame handleGetGPSLatitude(const std::string& param, OperationType operationType);
Frame handleGetGPSLatitudeDirection(const std::string& param, OperationType operationType);
Frame handleGetGPSLongitude(const std::string& param, OperationType operationType);
Frame handleGetGPSLongitudeDirection(const std::string& param, OperationType operationType);
Frame handleGetGPSSpeedOverGround(const std::string& param, OperationType operationType);
Frame handleGetGPSCourseOverGround(const std::string& param, OperationType operationType);
Frame handleGetGPSDate(const std::string& param, OperationType operationType);


// POWER
Frame handleGetPowerManagerIDs(const std::string& param, OperationType operationType);
Frame handleGetVoltageBattery(const std::string& param, OperationType operationType);
Frame handleGetVoltage5V(const std::string& param, OperationType operationType);
Frame handleGetCurrentChargeUSB(const std::string& param, OperationType operationType);
Frame handleGetCurrentChargeSolar(const std::string& param, OperationType operationType);
Frame handleGetCurrentChargeTotal(const std::string& param, OperationType operationType);
Frame handleGetCurrentDraw(const std::string& param, OperationType operationType);


Frame executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType);
extern std::map<uint32_t, std::function<Frame(const std::string&, OperationType)>> commandHandlers;

#endif