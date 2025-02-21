// commands/commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <functional>
#include <map>
#include "protocol.h"


// CLOCK
Frame handle_time(const std::string& param, OperationType operationType);
Frame handle_timezone_offset(const std::string& param, OperationType operationType);
Frame handle_clock_sync_interval(const std::string& param, OperationType operationType);
Frame handle_get_last_sync_time(const std::string& param, OperationType operationType);


// DIAG
Frame handle_get_commands_list(const std::string& param, OperationType operationType);
Frame handle_get_build_version(const std::string& param, OperationType operationType);
Frame handle_verbosity(const std::string& param, OperationType operationType);
Frame handle_enter_bootloader_mode(const std::string& param, OperationType operationType);


// GPS
Frame handle_gps_power_status(const std::string& param, OperationType operationType);
Frame handle_enable_gps_uart_passthrough(const std::string& param, OperationType operationType);
Frame handle_get_rmc_data(const std::string& param, OperationType operationType);
Frame handle_get_gga_data(const std::string& param, OperationType operationType);


// POWER
Frame handle_get_power_manager_ids(const std::string& param, OperationType operationType);
Frame handle_get_voltage_battery(const std::string& param, OperationType operationType);
Frame handle_get_voltage_5v(const std::string& param, OperationType operationType);
Frame handle_get_current_charge_usb(const std::string& param, OperationType operationType);
Frame handle_get_current_charge_solar(const std::string& param, OperationType operationType);
Frame handle_get_current_charge_total(const std::string& param, OperationType operationType);
Frame handle_get_current_draw(const std::string& param, OperationType operationType);


// EVENT
Frame handle_get_last_events(const std::string& param, OperationType operationType);
Frame handle_get_event_count(const std::string& param, OperationType operationType);


//STORAGE
Frame handle_list_files(const std::string& param, OperationType operationType);
Frame handle_file_download(const std::string& param, OperationType operationType);
Frame handle_mount(const std::string& param, OperationType operationType);

Frame execute_command(uint32_t commandKey, const std::string& param, OperationType operationType);
extern std::map<uint32_t, std::function<Frame(const std::string&, OperationType)>> commandHandlers;

#endif