#include "protocol.h"
#include "LoRa-RP2040.h"
#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <map>

using std::string;

// Global variables
string outgoing;
uint8_t msgCount = 0;
long lastSendTime = 0;
long lastReceiveTime = 0;
long lastPrintTime = 0;
unsigned long interval = 0;

// Initialize radio hardware
bool initializeRadio() {
    LoRa.setPins(csPin, resetPin, irqPin);
    long frequency = 433E6;
    if (!LoRa.begin(frequency))
    {
        logMessage("LoRa init failed. Check your connections.");
        return false;
    }
    logMessage("LoRa initialized with frequency " + std::to_string(frequency));
    return true;
}

void sendMessage(string outgoing)
{
    int n = outgoing.length();
    char send[n + 1];
    strcpy(send, outgoing.c_str());
    logMessage("Sat (0x" + std::to_string(localAddress) + ") to ground(" + std::to_string(destination) + "): " + string(send) + " [" + std::to_string(n) + "]");

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
    LoRa.flush();
}

// Simple logging function with timestamp printing.
void logMessage(const string &message)
{
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    printf("[%lu ms] %s\n", timestamp, message.c_str());
}

// Helper: send a large packet via LoRa in chunks.
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// FRAME BUILDER: creates a frame from given parameters.
Frame buildFrame(uint8_t direction, uint8_t operation, uint8_t group, uint8_t command, const std::vector<uint8_t>& payload) {
    Frame frame;
    frame.direction = direction;
    frame.operation = operation;
    frame.group = group;
    frame.command = command;
    frame.payload = payload;
    frame.length = payload.size();
    // Calculate simple checksum: sum bytes from direction to last payload byte.
    uint8_t checksum = 0;
    checksum += frame.direction;
    checksum += frame.operation;
    checksum += frame.group;
    checksum += frame.command;
    checksum += (frame.length >> 8) & 0xFF;
    checksum += frame.length & 0xFF;
    for (auto byte : frame.payload)
        checksum += byte;
    frame.checksum = checksum;
    return frame;
}

// Encode a frame into a byte buffer
std::vector<uint8_t> encodeFrame(const Frame& frame)
{
    std::vector<uint8_t> buffer;
    buffer.push_back(frame.header);
    buffer.push_back(frame.direction);
    buffer.push_back(frame.operation);
    buffer.push_back(frame.group);
    buffer.push_back(frame.command);

    // Add 16-bit payload length
    buffer.push_back((frame.length >> 8) & 0xFF);
    buffer.push_back(frame.length & 0xFF);

    // Add payload bytes
    buffer.insert(buffer.end(), frame.payload.begin(), frame.payload.end());

    // Append checksum
    buffer.push_back(frame.checksum);
    return buffer;
}

// Decode a byte buffer into a Frame. Throws std::runtime_error if frame is bad.
Frame decodeFrame(const std::vector<uint8_t>& data)
{
    if (data.size() < 8)
        throw std::runtime_error("Data too short to be a valid frame");

    Frame frame;
    size_t pos = 0;
    frame.header = data[pos++];
    if (frame.header != 0xCAFE)
        throw std::runtime_error("Invalid frame header");

    frame.direction = data[pos++];
    frame.operation = data[pos++];
    frame.group = data[pos++];
    frame.command = data[pos++];

    frame.length = (data[pos] << 8) | data[pos + 1];
    pos += 2;

    if (data.size() < pos + frame.length + 1)
        throw std::runtime_error("Data length does not match payload length");

    frame.payload.assign(data.begin() + pos, data.begin() + pos + frame.length);
    pos += frame.length;
    frame.checksum = data[pos];

    // Recalculate checksum
    uint8_t calcSum = 0;
    calcSum += frame.direction;
    calcSum += frame.operation;
    calcSum += frame.group;
    calcSum += frame.command;
    calcSum += (frame.length >> 8) & 0xFF;
    calcSum += frame.length & 0xFF;
    for (auto byte : frame.payload)
        calcSum += byte;

    if (calcSum != frame.checksum)
        throw std::runtime_error("Checksum mismatch");

    return frame;
}

// Sends a Frame using LoRa by encoding it into bytes.
void sendFrame(const Frame& frame) {
    std::vector<uint8_t> buffer = encodeFrame(frame);
    sendLargePacket(buffer.data(), buffer.size());
}

// Command handler function type
using CommandHandler = std::function<void(const std::string&)>;

// Declare command map
extern std::map<uint32_t, CommandHandler> commandHandlers;

// Once a frame is decoded, call the command handler to execute it.
void handleCommandFrame(const Frame& frame) {
    // Combine group and command IDs into a single 32-bit key
    uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);

    // Look up the command handler in the map
    auto it = commandHandlers.find(commandKey);
    if (it != commandHandlers.end()) {
        // Extract the payload as a string
        std::string param(frame.payload.begin(), frame.payload.end());

        // Call the command handler with the payload
        it->second(param);
    } else {
        logMessage("Error: Unknown group/command combination");
    }
}

// Sample onReceive callback that decodes a received packet into a frame.
void onReceive(int packetSize)
{
    if(packetSize == 0)
        return;

    // Read raw bytes from LoRa into a string first
    std::string hexString;
    while(LoRa.available()) {
        hexString += (char)LoRa.read();
    }

    try {
        // Debug print received hex string
        uart_puts(DEBUG_UART_PORT, "Received hex: ");
        uart_puts(DEBUG_UART_PORT, hexString.c_str());
        uart_puts(DEBUG_UART_PORT, "\r\n");

        // Convert hex string to bytes
        std::vector<uint8_t> buffer;
        for(size_t i = 0; i < hexString.length(); i += 2) {
            std::string byteString = hexString.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            buffer.push_back(byte);
        }

        // Debug print converted bytes
        std::string hexStr = "Converted bytes: ";
        for(uint8_t b : buffer) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X ", b);
            hexStr += hex;
        }
        uart_puts(DEBUG_UART_PORT, hexStr.c_str());
        uart_puts(DEBUG_UART_PORT, "\r\n");

        Frame frame = decodeFrame(buffer);
        logMessage("Received valid frame from group: " + std::to_string(frame.group) +
                   " command: " + std::to_string(frame.command));
        handleCommandFrame(frame);
    }
    catch(const std::exception& e) {
        logMessage("Frame error: " + std::string(e.what()));
    }

    lastReceiveTime = to_ms_since_boot(get_absolute_time());
}