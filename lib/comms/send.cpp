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


/**
 * @brief Sends a Frame using LoRa.
 * @param frame The Frame to send.
 * @details Encodes the Frame into a string and sends it using the sendMessage function.
 */
void sendFrame(const Frame& frame) {
    
    std::string encodedFrame = encodeFrame(frame);
    // sendLargePacket(data, encodedFrame);
    sendMessage(encodedFrame);
}


/**
 * @brief Sends a large packet using LoRa.
 * @param data The data to send.
 * @param length The length of the data.
 * @details Splits the data into chunks of MAX_PKT_SIZE and sends each chunk as a separate LoRa packet.
 */
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