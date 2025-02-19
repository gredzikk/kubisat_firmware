#include "communication.h"


/**
 * @file receive.cpp
 * @brief Implements functions for receiving and processing data, including LoRa and UART input.
 */

/**
 * @brief Callback function for handling received LoRa packets.
 * @param packetSize The size of the received packet.
 * @details Reads the received LoRa packet, extracts metadata, validates the lora_address_remote and local addresses,
 *          extracts the frame data, and processes it. Prints raw hex values for debugging.
 */
void on_receive(int packetSize) {
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
    if (receivedDestination != lora_address_local) {
        uart_print("Error: Destination address mismatch!");
        return;
    }
    
    if (receivedLocalAddress != lora_address_remote) {
        uart_print("Error: Local address mismatch!");
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
    uart_print(hexDump.str());
    
    // Find frame boundaries
    size_t headerPos = received.find(FRAME_BEGIN);
    size_t footerPos = received.find(FRAME_END);
    
    if (headerPos != std::string::npos && footerPos != std::string::npos && footerPos > headerPos) {
        // Extract frame between header and footer
        std::string frameData = received.substr(headerPos, footerPos + FRAME_END.length() - headerPos);
        uart_print("Extracted frame (length=" + std::to_string(frameData.length()) + "): " + frameData);
        frame_process(frameData);
    } else {
        uart_print("No valid frame found in received data");
    }
}


/**
 * @brief Handles UART input.
 * @details Reads characters from the UART port, appends them to a buffer, and processes the buffer when a newline
 *          character is received.
 */
void handle_uart_input() {
    static std::string uartBuffer; // Static buffer to store UART input

    while (uart_is_readable(DEBUG_UART_PORT)) {
        char c = uart_getc(DEBUG_UART_PORT);

        if (c == '\r' || c == '\n') {
            uart_print("Received UART string: " + uartBuffer);
            frame_process(uartBuffer); // Process the data
            uartBuffer.clear(); // Clear the buffer for the next input
        } else {
            // Append the character to the buffer
            uartBuffer += c;
        }
    }
}