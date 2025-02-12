// communication.cpp
#include "protocol.h"
#include "LoRa-RP2040.h"
#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <map>
#include "utils.h"
#include <sstream>
#include <iomanip>

using std::string;

string outgoing;
uint8_t msgCount = 0;
long lastSendTime = 0;
long lastReceiveTime = 0;
long lastPrintTime = 0;
unsigned long interval = 0;

bool initializeRadio() {
    LoRa.setPins(csPin, resetPin, irqPin);
    long frequency = 433E6;
    if (!LoRa.begin(frequency))
    {
        uartPrint("LoRa init failed. Check your connections.");
        return false;
    }
    uartPrint("LoRa initialized with frequency " + std::to_string(frequency));
    return true;
}

void sendMessage(string outgoing)
{
    int n = outgoing.length();
    char send[n + 1];
    strcpy(send, outgoing.c_str());

    LoRa.beginPacket();       // start packet
    LoRa.write(destination);  // add destination address
    LoRa.write(localAddress); // add sender address
    LoRa.write(msgCount);     // add message ID
    LoRa.write(n + 1);        // add payload length
    LoRa.print(send);         // add payload
    LoRa.endPacket(false);    // finish packet and send it
    msgCount++;               // increment message ID

    std::string messageToLog = "Sent message of size " + std::to_string(n) + " with ID " + std::to_string(msgCount);
    messageToLog += " to: 0x" + std::to_string(destination);
    messageToLog += " containing: " + string(send);

    uartPrint(messageToLog);
    
    LoRa.flush();
}

void sendLargePacket(const uint8_t* data, size_t length)
{
    const size_t MAX_PKT_SIZE = 255;
    size_t offset = 0;
    while (offset < length)
    {
        size_t chunkSize = ((length - offset) < MAX_PKT_SIZE) ? (length - offset) : MAX_PKT_SIZE;
        LoRa.beginPacket();
        LoRa.write(&data[offset], chunkSize);
        LoRa.endPacket();
        offset += chunkSize;
        sleep_ms(100);
    }
}

// Function to convert OperationType to string
std::string operationTypeToString(OperationType type) {
    switch (type) {
        case OperationType::GET: return "GET";
        case OperationType::SET: return "SET";
        case OperationType::ANS: return "ANS";
        default: return "UNKNOWN";
    }
}

// Function to convert string to OperationType
OperationType stringToOperationType(const std::string& str) {
    if (str == "GET") return OperationType::GET;
    if (str == "SET") return OperationType::SET;
    if (str == "ANS") return OperationType::ANS;
    return OperationType::GET; // Default to GET
}

Frame buildFrame(uint8_t direction, OperationType operationType, uint8_t group, uint8_t command, const std::string& value, const std::string& unit) {
    Frame frame;
    frame.header = HEADER;
    frame.direction = direction;
    frame.operationType = operationType;
    frame.group = group;
    frame.command = command;
    frame.value = value;
    frame.unit = unit;
    return frame;
}

// Encode a frame into a string
std::string encodeFrame(const Frame& frame) {
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operationTypeToString(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value << DELIMITER
       << frame.unit;

    std::string frameData = ss.str();

    // Calculate CRC-16 checksum
    uint16_t checksum = crc16(reinterpret_cast<const uint8_t*>(frameData.c_str()), frameData.length());

    // Append checksum to the frame data
    ss << DELIMITER << std::hex << std::setw(4) << std::setfill('0') << checksum;

    return HEADER + ss.str();
}

// Decode a string into a Frame. Throws std::runtime_error if frame is bad.
Frame decodeFrame(const std::string& data) {
    Frame frame;
    std::stringstream ss(data);
    std::string token;

    std::getline(ss, token, DELIMITER);
    if (token != HEADER)
        throw std::runtime_error("Invalid frame header");
    frame.header = token;

    std::string frameDataWithoutCrc;
    while (std::getline(ss, token, DELIMITER)) {
        if (ss.tellg() == -1) break;
        frameDataWithoutCrc += token + DELIMITER;
    }
    if (!frameDataWithoutCrc.empty()) {
        frameDataWithoutCrc.pop_back();
    }

    uint16_t receivedCrc;
    std::stringstream crcStream(token);
    crcStream >> std::hex >> receivedCrc;

    uint16_t calculatedCrc = crc16(reinterpret_cast<const uint8_t*>(frameDataWithoutCrc.c_str()), frameDataWithoutCrc.length());

    if (receivedCrc != calculatedCrc) {
        throw std::runtime_error("CRC check failed");
    }

    std::stringstream frameDataStream(frameDataWithoutCrc);

    std::getline(frameDataStream, token, DELIMITER);
    frame.direction = std::stoi(token);

    std::getline(frameDataStream, token, DELIMITER);
    frame.operationType = stringToOperationType(token);

    std::getline(frameDataStream, token, DELIMITER);
    frame.group = std::stoi(token);

    std::getline(frameDataStream, token, DELIMITER);
    frame.command = std::stoi(token);

    std::getline(frameDataStream, token, DELIMITER);
    frame.value = token;

    std::getline(frameDataStream, token, DELIMITER);
    frame.unit = token;

    return frame;
}

Frame buildResponseFrame(const Frame& requestFrame, const std::string& value) {
    Frame responseFrame;
    responseFrame.header = HEADER;
    responseFrame.direction = (requestFrame.direction == 0) ? 1 : 0; // Reverse direction
    responseFrame.operationType = OperationType::ANS; // ANS
    responseFrame.group = requestFrame.group;
    responseFrame.command = requestFrame.command;
    responseFrame.value = value;

    // Find the unit from getGroups
    std::vector<Group> groups = getGroups();
    for (const auto& group : groups) {
        if (group.Id == requestFrame.group) {
            for (const auto& command : group.Commands) {
                if (command.Id == requestFrame.command) {
                    // Assign the unit to the response frame
                    switch (command.Unit) {
                        case ValueUnit::VOLT:
                            responseFrame.unit = "V";
                            break;
                        case ValueUnit::BOOL:
                            responseFrame.unit = "";
                            break;
                        case ValueUnit::DATETIME:
                            responseFrame.unit = "";
                            break;
                        case ValueUnit::SECOND:
                            responseFrame.unit = "s";
                            break;
                        case ValueUnit::MILIAMP:
                            responseFrame.unit = "mA";
                            break;
                        default:
                            responseFrame.unit = "";
                            break;
                    }
                    return responseFrame;
                }
            }
        }
    }

    responseFrame.unit = "";
    return responseFrame;
}

// Sends a Frame using LoRa by encoding it into bytes.
void sendFrame(const Frame& frame) {
    std::string encodedFrame = encodeFrame(frame);
    // sendLargePacket(data, encodedFrame);
    sendMessage(encodedFrame);
}

// Command handler function type
using CommandHandler = std::function<std::string(const std::string&)>;

// Declare command map
extern std::map<uint32_t, CommandHandler> commandHandlers;

// Once a frame is decoded, call the command handler to execute it.
void handleCommandFrame(const Frame& frame) {
    uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);
    auto it = commandHandlers.find(commandKey);

    if (it != commandHandlers.end()) {
        std::string param = frame.value;

        // Execute the command and get the response data
        std::string response = executeCommand(commandKey, param);

        uartPrint("Sending response data: " + response);
        Frame responseFrame = buildResponseFrame(frame, response);
        sendFrame(responseFrame);
    } else {
        uartPrint("Error: Unknown group/command combination");
    }
}

void processFrameData(const std::string& data) {
    try {
        // Find the starting position of the header
        size_t headerPos = data.find(HEADER);
        if (headerPos == std::string::npos) {
            throw std::runtime_error("Invalid frame header");
        }

        // Extract the frame data starting from the header
        std::string frameData = data.substr(headerPos);

        Frame frame = decodeFrame(frameData);
        std::string messageToLog = "Received valid frame from group: " + std::to_string(frame.group) +
                                   " command: " + std::to_string(frame.command);
        uartPrint(messageToLog);
        handleCommandFrame(frame);
    } catch (const std::exception& e) {
        uartPrint("Frame error: " + std::string(e.what()));
    }
}

void onReceive(int packetSize) {
    if (packetSize == 0)
        return;

    std::string receivedString;
    while (LoRa.available()) {
        receivedString += (char)LoRa.read();
    }

    uartPrint("Received LoRa string: " + receivedString, true);
    processFrameData(receivedString);
    lastReceiveTime = to_ms_since_boot(get_absolute_time());
}

void handleUartInput() {
    static std::string uartBuffer; // Static buffer to store UART input

    while (uart_is_readable(DEBUG_UART_PORT)) {
        char c = uart_getc(DEBUG_UART_PORT);

        if (c == '\r' || c == '\n') {
            uartPrint("Received UART string: " + uartBuffer);
            processFrameData(uartBuffer); // Process the data
            uartBuffer.clear(); // Clear the buffer for the next input
        } else {
            // Append the character to the buffer
            uartBuffer += c;
        }
    }
}

// Function to send the event register value via radio
extern volatile uint16_t eventRegister;
void sendEventRegister() {
    // Convert the event register value to a string
    std::stringstream ss;
    ss << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(eventRegister);
    std::string eventValue = ss.str();

    // Build the frame
    Frame eventFrame = buildFrame(
        1,                  // Direction: sat->ground
        OperationType::ANS, // Operation Type: ANS
        8,                  // Group ID: 8 (EVENTS)
        0,                  // Command ID: 0 (EVENT_REGISTER)
        eventValue,         // Value: event register value
        ""                  // Unit: None
    );

    // Send the frame
    sendFrame(eventFrame);
}