// Auto-generated file - DO NOT EDIT
#include "protocol.h"

std::vector<Group> getGroups()
{
    return {
        {
            0,
            "INFO",
            {
                { 0, "GENERIC_INFO", CommandAccessLevel::READ_ONLY, ValueUnit::UNDEFINED },
            }
        },
        {
            1,
            "DIAGNOSTICS",
            {
                { 0, "FIRMWARE_VERSION", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "BUILD_VERSION", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 2, "BUILD_DATE", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
            }
        },
        {
            2,
            "POWER_MANAGER",
            {
                { 0, "ID", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "NDEF", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 2, "VOLTAGE_BATTERY", CommandAccessLevel::READ_ONLY, ValueUnit::VOLT },
                { 3, "VOLTAGE_5V", CommandAccessLevel::READ_ONLY, ValueUnit::VOLT },
                { 4, "CURRENT_CHARGE_USB", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 5, "CURRENT_CHARGE_SOLAR", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 6, "CURRENT_CHARGE_TOTAL", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
                { 7, "CURRENT_DRAW", CommandAccessLevel::READ_ONLY, ValueUnit::MILIAMP },
            }
        },
        {
            3,
            "CLOCK",
            {
                { 0, "CURRENT_TIME", CommandAccessLevel::READ_WRITE, ValueUnit::DATETIME },
                { 1, "GPS_TIME_SYNC", CommandAccessLevel::READ_WRITE, ValueUnit::BOOL },
                { 2, "GPS_TIME_SYNC_INTERVAL", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND },
                { 3, "TIMEZONE", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND },
            }
        },
        {
            4,
            "RADIO",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            6,
            "STORAGE",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            7,
            "GPS",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 1, "POWER_STATUS", CommandAccessLevel::READ_WRITE, ValueUnit::BOOL },
                { 2, "TRANSPARENT_MODE_TIMEOUT", CommandAccessLevel::READ_WRITE, ValueUnit::SECOND },
                { 3, "DATA", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
            }
        },
        {
            8,
            "EVENTS",
            {
                { 0, "EVENT_REGISTER", CommandAccessLevel::READ_ONLY, ValueUnit::TEXT },
                { 1, "RESET_EVENTS", CommandAccessLevel::WRITE_ONLY, ValueUnit::BOOL },
            }
        },
        {
            9,
            "MAGNETOMETER",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            10,
            "ENVIRONMENTAL_SENSOR",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            11,
            "LIGHT_SENSOR",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            12,
            "6DOF_IMU",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            13,
            "GASP",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
            }
        },
        {
            14,
            "RESTART",
            {
                { 0, "RESERVED", CommandAccessLevel::NONE, ValueUnit::UNDEFINED },
                { 1, "RESTART_BOOTSEL", CommandAccessLevel::WRITE_ONLY, ValueUnit::UNDEFINED },
            }
        },
    };
}