#pragma once
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <cstdint>

enum class DataTypeIdentifier
{
    INT8 = 0x01,
    UINT8 = 0x02,
    INT16 = 0x03,
    UINT16 = 0x04,
    INT32 = 0x05,
    UINT32 = 0x06,
    FLOAT = 0x07,
    DOUBLE = 0x08,
    BOOL = 0x09,
    STRING = 0x0A,
    DATETIME = 0x0B,
    UNDEFINED = 0xFF
};

enum class CommandAccessLevel : uint8_t
{
    NONE = 0x00,
    READ_ONLY = 0x01,
    WRITE = 0x02,
    READ_WRITE = 0x03
};
struct Command
{
    int Id;
    std::string Name;
    CommandAccessLevel AccessRights;
    DataTypeIdentifier DataType;
};

struct Group
{
    int Id;
    std::string Name;
    std::vector<Command> Commands;
};


struct Frame {
    int header = 0xCAFE;    // Start marker
    uint8_t direction;        // 0 = ground->sat, 1 = sat->ground
    uint8_t operation;        // 0 = get, 1 = set
    uint8_t group;            // Group ID
    uint8_t command;          // Command ID within group
    uint16_t length;          // Payload length
    std::vector<uint8_t> payload;
    uint8_t checksum;         // Simple checksum
};

bool initializeRadio();
void logMessage(const std::string &message);
void sendMessage(std::string outgoing);
void sendLargePacket(const uint8_t* data, size_t length);
void onReceive(int packetSize);

Frame buildFrame(uint8_t direction, uint8_t operation, uint8_t group, uint8_t command, const std::vector<uint8_t>& payload);
std::vector<uint8_t> encodeFrame(const Frame& frame);
Frame decodeFrame(const std::vector<uint8_t>& data);
void handleCommandFrame(const Frame& frame);

using CommandHandler = std::function<void(const std::string&)>;

extern std::map<uint32_t, CommandHandler> commandHandlers;

inline std::vector<Group> getGroups()
{
    return {
        {
            0,
            "0x00 - TEST",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            1,
            "0x01 - DIAGNOSTICS",
            {
                { 0, "0x00 - FIRMWARE_VERSION", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::UINT32 },
            }
        },
        {
            2,
            "0x02 - POWER MANAGER",
            {
                { 0, "0x00 - ID", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::UINT32 },
                { 1, "0x01 - NDEF", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED },
                { 2, "0x02 - VOLTAGE_BATTERY", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
                { 3, "0x03 - VOLTAGE_5V", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
                { 4, "0x04 - CURRENT_CHARGE_USB", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
                { 5, "0x05 - CURRENT_CHARGE_SOLAR", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
                { 6, "0x06 - CURRENT_CHARGE_TOTAL", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
                { 7, "0x07 - CURRENT_DRAW", CommandAccessLevel::READ_ONLY, DataTypeIdentifier::FLOAT },
            }
        },
        {
            3,
            "0x03 - CLOCK",
            {
                { 0, "0x00 - CURRENT_TIME", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::DATETIME },
                { 1, "0x01 - GPS_TIME_SYNC", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::BOOL },
                { 2, "0x02 - GPS_TIME_SYNC_INTERVAL", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::UINT32 },
                { 3, "0x03 - TIMEZONE", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::INT8 }
            }
        },
        {
            4,
            "0x04 - RADIO",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            6,
            "0x06 - STORAGE",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            7,
            "0x07 - GPS",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED },
                { 1, "0x01 - POWER_STATUS", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::BOOL },
                { 2, "0x02 - TRANSPARENT_MODE", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::BOOL },
                { 3, "0x03 - TRANSPARENT_MODE_TIMEOUT", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::UINT32 }
            }
        },
        {
            9,
            "0x09 - SERIAL",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            10,
            "0x0A - MAGNETOMETER",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            11,
            "0x0B - ENVIRONMENTAL_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            12,
            "0x0C - LIGHT_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            13,
            "0x0D - 6DOF_IMU",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            14,
            "0x0E - KUBISAT_INFO",
            {
                { 0, "0x00 - NAME", CommandAccessLevel::READ_WRITE, DataTypeIdentifier::STRING },
            }
        },
        {
            15,
            "0x0F - SX1278_INFO",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            16,
            "0x10 - LOG",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            17,
            "0x11 - GASP",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        },
        {
            18,
            "0x12 - RESTART",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, DataTypeIdentifier::UNDEFINED }
            }
        }
    };
}
