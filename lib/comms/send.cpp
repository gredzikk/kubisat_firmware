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
    LoRa.endPacket(true);    // finish packet and send it

    uart_print("LoRa packet end", VerbosityLevel::DEBUG);

    std::string message_to_log = "Sent message of size " + std::to_string(n);
    message_to_log += " to 0x" + std::to_string(lora_address_remote);
    message_to_log += " containing: " + string(send);

    uart_print(message_to_log, VerbosityLevel::DEBUG);
    
    LoRa.flush();
}


void send_frame_lora(const Frame& frame) {
    uart_print("Sending frame via LoRa", VerbosityLevel::DEBUG);
    string outgoing = frame_encode(frame);
    send_message(outgoing);
    uart_print("Frame sent via LoRa", VerbosityLevel::DEBUG);
}

void send_frame_uart(const Frame& frame) {
    std::string encoded_frame = frame_encode(frame);
    uart_print(encoded_frame);
}


/**
 * @brief Sends a large packet using LoRa.
 * @param data The data to send.
 * @param length The length of the data.
 * @details Splits the data into chunks of MAX_PKT_SIZE and sends each chunk as a separate LoRa packet.
 */
void split_and_send_message(const uint8_t* data, size_t length)
{
    const size_t MAX_PKT_SIZE = 255;
    size_t offset = 0;
    while (offset < length)
    {
        size_t chunk_size = ((length - offset) < MAX_PKT_SIZE) ? (length - offset) : MAX_PKT_SIZE;
        LoRa.beginPacket();
        LoRa.write(&data[offset], chunk_size);
        LoRa.endPacket();
        offset += chunk_size;
        sleep_ms(100);
    }
}

