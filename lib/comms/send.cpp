#include "communication.h"


/**
 * @file send.cpp
 * @brief Implements functions for sending data, including LoRa messages and Frames.
 */

/**
 * @brief Sends a message using LoRa.
 * @param outgoing The message to send.
 * @details Converts the outgoing string to a C-style string, adds destination and local addresses,
 *          and sends the message using LoRa. Prints a log message to the UART.
 */
void send_message(string outgoing)
{
    int n = outgoing.length();
    char send[n + 1];
    strcpy(send, outgoing.c_str());

    uart_print("LoRa packet begin", VerbosityLevel::DEBUG);
    LoRa.beginPacket();       // start packet
    LoRa.write(lora_address_remote);  // add destination address
    LoRa.write(lora_address_local); // add sender address
    LoRa.print(send);         // add payload
    LoRa.endPacket(false);    // finish packet and send it

    uart_print("LoRa packet end", VerbosityLevel::DEBUG);

    std::string message_to_log = "Sent message of size " + std::to_string(n);
    message_to_log += " to 0x" + std::to_string(lora_address_remote);
    message_to_log += " containing: " + string(send);

    uart_print(message_to_log, VerbosityLevel::DEBUG);
    
    LoRa.flush();
}


void send_frame_lora(const Frame& frame) {
    if (LoRa.beginPacket()) {
        std::string encoded_frame = frame_encode(frame);
        LoRa.write((uint8_t*)encoded_frame.c_str(), encoded_frame.length());
        
        if (LoRa.endPacket()) {
            uart_print("LoRa frame sent: " + encoded_frame, VerbosityLevel::DEBUG);
        } else {
            uart_print("Failed to send LoRa frame", VerbosityLevel::ERROR);
            EventEmitter::emit(EventGroup::COMMS, CommsEvent::RADIO_ERROR);
        }
    } else {
        uart_print("Failed to begin LoRa packet", VerbosityLevel::ERROR);
        EventEmitter::emit(EventGroup::COMMS, CommsEvent::RADIO_ERROR);
    }
}

void send_frame_uart(const Frame& frame) {
    std::string encoded_frame = frame_encode(frame);
    uart_print(encoded_frame);
}


