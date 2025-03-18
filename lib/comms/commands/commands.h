// commands/commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <functional>
#include <map>
#include "protocol.h"

// CLOCK
std::vector<Frame> handle_time(const std::string& param, OperationType operationType);
std::vector<Frame> handle_timezone_offset(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_internal_temperature(const std::string& param, OperationType operationType);


// DIAG
std::vector<Frame> handle_get_commands_list(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_build_version(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_power_mode(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_uptime(const std::string& param, OperationType operationType);
std::vector<Frame> handle_verbosity(const std::string& param, OperationType operationType);
std::vector<Frame> handle_enter_bootloader_mode(const std::string& param, OperationType operationType);


// GPS
std::vector<Frame> handle_gps_power_status(const std::string& param, OperationType operationType);
std::vector<Frame> handle_enable_gps_uart_passthrough(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_rmc_data(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_gga_data(const std::string& param, OperationType operationType);


// POWER
std::vector<Frame> handle_get_power_manager_ids(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_voltage_battery(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_voltage_5v(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_current_charge_usb(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_current_charge_solar(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_current_charge_total(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_current_draw(const std::string& param, OperationType operationType);


// EVENT
std::vector<Frame> handle_get_last_events(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_event_count(const std::string& param, OperationType operationType);


//STORAGE
std::vector<Frame> handle_list_files(const std::string& param, OperationType operationType);
std::vector<Frame> handle_mount(const std::string& param, OperationType operationType);

// TELEMETRY
std::vector<Frame> handle_get_last_telemetry_record(const std::string& param, OperationType operationType);
std::vector<Frame> handle_get_last_sensor_record(const std::string& param, OperationType operationType);

std::vector<Frame> execute_command(uint32_t commandKey, const std::string& param, OperationType operationType);
extern std::map<uint32_t, std::function<std::vector<Frame>(const std::string&, OperationType)>> command_handlers;

#endif