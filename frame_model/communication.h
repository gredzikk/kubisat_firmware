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


void handleCommandFrame(const Frame& frame);
Frame executeCommand(uint32_t commandKey, const std::string& param, OperationType operationType);
void sendEventRegister();


std::string encodeFrame(const Frame& frame);
Frame decodeFrame(const std::string& data);

Frame buildFrame(ExecutionResult result, uint8_t group, uint8_t command,const std::string& value, const Frame* requestFrame = nullptr);
std::string determineUnit(uint8_t group, uint8_t command);

void sendMessage(std::string outgoing);
void sendFrame(const Frame& frame);
void sendLargePacket(const uint8_t* data, size_t length);


void onReceive(int packetSize);
void handleUartInput();
void processFrameData(const std::string& data);
std::vector<uint8_t> hexStringToBytes(const std::string& hexString);


#endif