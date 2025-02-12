#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <cstdint>

const std::string FRAME_BEGIN = "KBST";
const std::string FRAME_END = "TSBK";
const char DELIMITER = ';';

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

std::string operationTypeToString(OperationType type);

// Function to convert string to OperationType
OperationType stringToOperationType(const std::string& str);

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

bool initializeRadio();
void sendMessage(std::string outgoing);
void sendLargePacket(const uint8_t* data, size_t length);
void onReceive(int packetSize);
void handleUartInput();
void processFrameData(const std::string& data);

Frame buildFrame(uint8_t direction, OperationType operationType, uint8_t group, uint8_t command, const std::string& value, const std::string& unit);
std::string encodeFrame(const Frame& frame);
Frame decodeFrame(const std::string& data);
void handleCommandFrame(const Frame& frame);
std::string executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType);
void sendEventRegister();
std::vector<uint8_t> hexStringToBytes(const std::string& hexString);
using CommandHandler = std::function<std::string(const std::string&, OperationType)>;
extern std::map<uint32_t, CommandHandler> commandHandlers;

inline std::vector<Group> getGroups()
{
    return {
        {
            0,
            "0x00 - TEST",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            1,
            "0x01 - DIAGNOSTICS",
            {
                { 0, "0x00 - FIRMWARE_VERSION", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
            }
        },
        {
            2,
            "0x02 - POWER MANAGER",
            {
                { 0, "0x00 - ID", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "0x01 - NDEF", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 2, "0x02 - VOLTAGE_BATTERY", CommandAccessLevel::READ_ONLY, ValueUnit::VOLT },
                { 3, "0x03 - VOLTAGE_5V", CommandAccessLevel::READ_ONLY, ValueUnit::VOLT },
                { 4, "0x04 - CURRENT_CHARGE_USB", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 5, "0x05 - CURRENT_CHARGE_SOLAR", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 6, "0x06 - CURRENT_CHARGE_TOTAL", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 7, "0x07 - CURRENT_DRAW", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
            }
        },
        {
            3,
            "0x03 - CLOCK",
            {
                { 0, "0x00 - CURRENT_TIME", CommandAccessLevel::READ_WRITE, ValueUnit::DATETIME },
                { 1, "0x01 - GPS_TIME_SYNC", CommandAccessLevel::READ_WRITE, ValueUnit::BOOL },
                { 2, "0x02 - GPS_TIME_SYNC_INTERVAL", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND },
                { 3, "0x03 - TIMEZONE", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND }
            }
        },
        {
            4,
            "0x04 - RADIO",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            6,
            "0x06 - STORAGE",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            7,
            "0x07 - GPS",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 1, "0x01 - POWER_STATUS", CommandAccessLevel::READ_WRITE, ValueUnit::BOOL },
                { 2, "0x02 - TRANSPARENT_MODE_TIMEOUT", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND },
                { 3, "0x03 - DATA", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT }
            }
        },
        {
            8,
            "0x08 - EVENTS",
            {
                { 0, "0x00 - EVENT_REGISTER", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "0x01 - RESET_EVENTS", CommandAccessLevel::WRITE_ONLY, ValueUnit::BOOL }
            }
        },
        {
            9,
            "0x09 - SERIAL",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            10,
            "0x0A - MAGNETOMETER",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            11,
            "0x0B - ENVIRONMENTAL_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            12,
            "0x0C - LIGHT_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            13,
            "0x0D - 6DOF_IMU",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            14,
            "0x0E - KUBISAT_INFO",
            {
                { 0, "0x00 - NAME", CommandAccessLevel::READ_WRITE, ValueUnit::TEXT },
            }
        },
        {
            15,
            "0x0F - SX1278_INFO",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            16,
            "0x10 - LOG",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            17,
            "0x11 - GASP",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            18,
            "0x12 - RESTART",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        }
    };
}

#endif