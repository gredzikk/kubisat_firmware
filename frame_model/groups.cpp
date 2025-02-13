#include "protocol.h"

std::vector<Group> getGroups()
{
    return {
        {
            1,
            "0x01 - DIAGNOSTICS",
            {
                { 0, "0x00 - FIRMWARE_VERSION", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "0x01 - BUILD VERSION", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 2, "0x02 - BUILD_DATE", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT }
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
            "0x09 - MAGNETOMETER",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            10,
            "0x0A - ENVIRONMENTAL_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            11,
            "0x0B - LIGHT_SENSOR",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            12,
            "0x0C - 6DOF_IMU",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            13,
            "0x0D - GASP",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED }
            }
        },
        {
            14,
            "0x0E - RESTART",
            {
                { 0, "0x00 - RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 1, "0x01 - RESTART_BOOTSEL", CommandAccessLevel::WRITE_ONLY, ValueUnit::UNDEFINED }
            }
        }
    };
}
