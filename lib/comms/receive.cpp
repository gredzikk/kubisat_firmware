#include "communication.h"


/**
 * @file receive.cpp
 * @brief Implements functions for receiving and processing data, including LoRa and UART input.
 */

/**
 * @brief Callback function for handling received LoRa packets.
 * @param packetSize The size of the received packet.
 * @details Reads the received LoRa packet, extracts metadata, validates the destination and local addresses,
 *          extracts the frame data, and processes it. Prints raw hex values for debugging.
 */
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


/**
 * @brief Handles UART input.
 * @details Reads characters from the UART port, appends them to a buffer, and processes the buffer when a newline
 *          character is received.
 */
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