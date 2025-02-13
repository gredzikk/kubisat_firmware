// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <cstring>
#include "utils.h"
#include "time.h"
#include "gps_data.h"
#include "build_number.h"
#include "LoRa-RP2040.h"

const std::string FRAME_BEGIN = "KBST";
const std::string FRAME_END = "TSBK";
const char DELIMITER = ';';

enum class ExecutionResult {
    SUCCESS,
    ERROR,
    RESPONSE
};

enum class OperationType {
    GET,
    SET,
    ANS,
    ERR
};

enum class CommandAccessLevel {
    NONE,
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

enum class ValueUnit {
    UNDEFINED,
    SECOND,
    VOLT,
    BOOL,
    DATETIME,
    TEXT,
    MILIAMP,
};

enum class ExceptionType {
    NONE, 
    NOT_ALLOWED,
    INVALID_PARAM,
    INVALID_OPERATION,
    PARAM_UNECESSARY
};

struct Command
{
    int Id;
    std::string Name;
    CommandAccessLevel AccessRights;
    ValueUnit Unit;
};

struct Group
{
    int Id;
    std::string Name;
    std::vector<Command> Commands;
};

struct Frame {
    std::string header;    // Start marker
    uint8_t direction;        // 0 = ground->sat, 1 = sat->ground
    OperationType operationType;
    uint8_t group;            // Group ID
    uint8_t command;          // Command ID within group
    std::string value;          // Payload value
    std::string unit;           // Payload unit
    std::string footer;      // End marker
};

std::string exceptionTypeToString(ExceptionType type);

std::string operationTypeToString(OperationType type);
OperationType stringToOperationType(const std::string& str);

std::vector<Group> getGroups();

#endif