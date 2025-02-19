#include "communication.h"
#include "storage_commands_utils.h"

// Helper function to calculate checksum (simple XOR)
uint32_t calculate_checksum(const uint8_t* data, size_t length) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}


void send_data_block(const uint8_t* data, size_t length) {
    LoRa.beginPacket();
    LoRa.write(data, length);
    LoRa.endPacket();
}

// Receiving an ACK (simplified)
bool receive_ack() {
    // Implement logic to receive an ACK frame from the ground station
    // Return true if ACK received, false otherwise
    // This is a placeholder, replace with your actual ACK receiving logic
    return true; // Placeholder: Always return true for now
}