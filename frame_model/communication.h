// communication/communication.h
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <vector>
#include "protocol.h"

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
Frame executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType);
void sendEventRegister();
// Function declarations for encoding.cpp
std::string encodeFrame(const Frame& frame);
Frame decodeFrame(const std::string& data);

// Function declarations for sending.cpp
void sendMessage(std::string outgoing);
void sendFrame(const Frame& frame);
void sendLargePacket(const uint8_t* data, size_t length);

// Function declarations for receiving.cpp
void onReceive(int packetSize);
void handleUartInput();
void processFrameData(const std::string& data);
std::vector<uint8_t> hexStringToBytes(const std::string& hexString);

#endif