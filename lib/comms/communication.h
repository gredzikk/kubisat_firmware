#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <vector>
#include "protocol.h"
#include "event_manager.h"

bool initialize_radio();
void lora_tx_done_callback();
void on_receive(int packetSize);
void handle_uart_input();
void send_message(std::string outgoing);
void send_frame_uart(const Frame& frame);
void send_frame_lora(const Frame& frame);

void split_and_send_message(const uint8_t* data, size_t length);

std::vector<Frame> execute_command(uint32_t commandKey, const std::string& param, OperationType operationType);

void frame_process(const std::string& data, Interface interface);
std::string frame_encode(const Frame& frame);
Frame frame_decode(const std::string& data);
Frame frame_build(OperationType operation, uint8_t group, uint8_t command,const std::string& value, const ValueUnit unitType  = ValueUnit::UNDEFINED);

std::string determine_unit(uint8_t group, uint8_t command);

#endif