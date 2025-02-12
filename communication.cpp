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
    LoRa.print(send);         // add payload
    LoRa.endPacket(false);    // finish packet and send it

    std::string messageToLog = "Sent message of size " + std::to_string(n);
    messageToLog += " to 0x" + std::to_string(destination);
    messageToLog += " containing: " + string(send);

    uartPrint(messageToLog);
    
    LoRa.flush();
}

// Sends a Frame using LoRa by encoding it into bytes.
void sendFrame(const Frame& frame) {
    std::string encodedFrame = encodeFrame(frame);
    // sendLargePacket(data, encodedFrame);
    sendMessage(encodedFrame);
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
    frame.header = FRAME_BEGIN;
    frame.direction = direction;
    frame.operationType = operationType;
    frame.group = group;
    frame.command = command;
    frame.value = value;
    frame.unit = unit;
    frame.footer = FRAME_END;
    return frame;
}

Frame buildErrorFrame(const std::string& errorMessage) {
    return buildFrame(1, OperationType::ERR, 0, 0, errorMessage, "");
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

    return FRAME_BEGIN + DELIMITER + ss.str() + DELIMITER + FRAME_END;
}

// Decode a string into a Frame. Throws std::runtime_error if frame is bad.
Frame decodeFrame(const std::string& data) {
    try {
        Frame frame;
        std::stringstream ss(data);
        std::string token;

        std::getline(ss, token, DELIMITER);
        if (token != FRAME_BEGIN)
            throw std::runtime_error("Invalid frame header");
        frame.header = token;

        std::string frameDataWithoutCrc;
        while (std::getline(ss, token, DELIMITER)) {
            if (token == FRAME_END) break; // Stop at the footer
            frameDataWithoutCrc += token + DELIMITER;
        }
        if (!frameDataWithoutCrc.empty()) {
            frameDataWithoutCrc.pop_back();
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
    } catch (const std::exception& e) {
        uartPrint("Frame error: " + std::string(e.what()));
        Frame errorFrame = buildErrorFrame(e.what());
        sendFrame(errorFrame);
        throw; // Re-throw the exception so that the calling function knows that an error occurred.
    }
}
Frame buildResponseFrame(const Frame& requestFrame, const std::string& value) {
    Frame responseFrame;
    responseFrame.header = FRAME_BEGIN;
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



// Command handler function type
using CommandHandler = std::function<std::string(const std::string&, OperationType)>;

// Declare command map
extern std::map<uint32_t, CommandHandler> commandHandlers;

// Once a frame is decoded, call the command handler to execute it.
void handleCommandFrame(const Frame& frame) {
    uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);
    auto it = commandHandlers.find(commandKey);

    if (it != commandHandlers.end()) {
        std::string param = frame.value;

        // Execute the command and get the response data
        std::string response = executeCommand(commandKey, param, frame.operationType);

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
        size_t headerPos = data.find(FRAME_BEGIN);
        if (headerPos == std::string::npos) {
            throw std::runtime_error("Invalid frame header");
        }

        // Extract the frame data starting from the header
        std::string frameData = data.substr(headerPos);

        // Find the footer position
        size_t footerPos = frameData.find(FRAME_END);

        // Extract the frame data without the footer
        std::string frameWithoutFooter = (footerPos != std::string::npos) ?
            frameData.substr(0, footerPos) : frameData;

        Frame frame = decodeFrame(frameWithoutFooter);
        std::string messageToLog = "Received valid frame from group: " + std::to_string(frame.group) +
                                   " command: " + std::to_string(frame.command);
        uartPrint(messageToLog);
        handleCommandFrame(frame);
    } catch (const std::exception& e) {
        uartPrint("Frame error: " + std::string(e.what()));
        Frame errorFrame = buildErrorFrame(e.what());
        sendFrame(errorFrame);
    }
}

std::vector<uint8_t> hexStringToBytes(const std::string& hexString) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::string byteString = hexString.substr(i, 2);
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << byteString;
        ss >> byte;
        bytes.push_back(static_cast<uint8_t>(byte));
    }
    return bytes;
}

void onReceive(int packetSize) {
    if (packetSize == 0) return;

    uint8_t buffer[256];
    int bytesRead = 0;
    
    while (LoRa.available() && bytesRead < packetSize) {
        buffer[bytesRead++] = LoRa.read();
    }
    
    // Extract LoRa metadata
    uint8_t receivedDestination = buffer[0];
    uint8_t receivedLocalAddress = buffer[1];
    
    // Validate metadata (optional, for security)
    if (receivedDestination != localAddress) {
        uartPrint("Error: Destination address mismatch!");
        return;
    }
    
    if (receivedLocalAddress != destination) {
        uartPrint("Error: Local address mismatch!");
        return;
    }

    // Find the starting index of the actual frame data
    int startIndex = 2; // Start after the metadata
    
    // Extract the frame data
    std::string received = std::string(reinterpret_cast<char*>(buffer + startIndex), bytesRead - startIndex);
    
    if (received.empty()) return;
    
    // Debug: Print raw hex values
    std::stringstream hexDump;
    hexDump << "Raw bytes: ";
    for (int i = 0; i < bytesRead; i++) {
        hexDump << std::hex << std::setfill('0') << std::setw(2) 
                << static_cast<int>(buffer[i]) << " ";
    }
    uartPrint(hexDump.str());
    
    // Find frame boundaries
    size_t headerPos = received.find(FRAME_BEGIN);
    size_t footerPos = received.find(FRAME_END);
    
    if (headerPos != std::string::npos && footerPos != std::string::npos && footerPos > headerPos) {
        // Extract frame between header and footer
        std::string frameData = received.substr(headerPos, footerPos + FRAME_END.length() - headerPos);
        uartPrint("Extracted frame (length=" + std::to_string(frameData.length()) + "): " + frameData);
        processFrameData(frameData);
    } else {
        uartPrint("No valid frame found in received data");
    }
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