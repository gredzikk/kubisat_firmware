#include "communication.h"


/**
 * @file receive.cpp
 * @brief Implements functions for receiving and processing data, including LoRa and UART input.
 */

/**
 * @brief Callback function for handling received LoRa packets.
 * @param packet_size The size of the received packet.
 * @details Reads the received LoRa packet, extracts metadata, validates the lora_address_remote and local addresses,
 *          extracts the frame data, and processes it. Prints raw hex values for debugging.
 */
void on_receive(int packet_size) {
    if (packet_size == 0) return;
    uart_print("Received LoRa packet of size " + std::to_string(packet_size), VerbosityLevel::DEBUG);

    uint8_t buffer[256];
    int bytes_read = 0;
    
    while (LoRa.available() && bytes_read < packet_size) {
        buffer[bytes_read++] = LoRa.read();
    }

    uart_print("Received " + std::to_string(bytes_read) + " bytes", VerbosityLevel::DEBUG);
    
    // Extract LoRa metadata
    uint8_t received_destination = buffer[0];
    uint8_t received_local_address = buffer[1];
    
    // Validate metadata (optional, for security)
    if (received_destination != lora_address_local) {
        uart_print("Error: Destination address mismatch!", VerbosityLevel::ERROR);
        return;
    }
    
    if (received_local_address != lora_address_remote) {
        uart_print("Error: Local address mismatch!", VerbosityLevel::ERROR);
        return;
    }

    // Find the starting index of the actual frame data
    int start_index = 2; // Start after the metadata
    
    // Extract the frame data
    std::string received = std::string(reinterpret_cast<char*>(buffer + start_index), bytes_read - start_index);
    
    if (received.empty()) return;
    
    // Debug: Print raw hex values
    std::stringstream hex_dump;
    hex_dump << "Raw bytes: ";
    for (int i = 0; i < bytes_read; i++) {
        hex_dump << std::hex << std::setfill('0') << std::setw(2) 
                << static_cast<int>(buffer[i]) << " ";
    }
    uart_print(hex_dump.str(), VerbosityLevel::DEBUG);
    
    // Find frame boundaries
    size_t header_pos = received.find(FRAME_BEGIN);
    size_t footer_pos = received.find(FRAME_END);
    
    if (header_pos != std::string::npos && footer_pos != std::string::npos && footer_pos > header_pos) {
        std::string frame_data = received.substr(header_pos, footer_pos + FRAME_END.length() - header_pos);
        uart_print("Extracted frame (length=" + std::to_string(frame_data.length()) + "): " + frame_data, VerbosityLevel::DEBUG);
        frame_process(frame_data, Interface::LORA);  
    } else {
        uart_print("No valid frame found in received data", VerbosityLevel::WARNING);
    }
}


/**
 * @brief Handles UART input.
 * @details Reads characters from the UART port, appends them to a buffer, and processes the buffer when a newline
 *          character is received.
 */
void handle_uart_input() {
    static std::string uart_buffer;

    while (uart_is_readable(DEBUG_UART_PORT)) {
        char c = uart_getc(DEBUG_UART_PORT);

        if (c == '\r' || c == '\n') {
            uart_print("Received UART string: " + uart_buffer, VerbosityLevel::DEBUG);
            frame_process(uart_buffer, Interface::UART); 
            uart_buffer.clear();
        } else {
            uart_buffer += c;
        }
    }
}