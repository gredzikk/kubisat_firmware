// communication.cpp
#include "LoRa-RP2040.h"
#include "commands.h"
#include "communication.h"

#define MAX_PKT_SIZE 255

using std::string;

std::string outgoing;                // outgoing message
uint8_t msgCount = 0;                // count of outgoing messages
long lastSendTime = 0;               
long lastReceiveTime = 0;            
long lastPrintTime = 0;  

/**
 * @brief Initializes the LoRa radio hardware and sets frequency.
 * @return True if successful, otherwise false.
 */
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

/**
 * @brief Logs a message with a timestamp to stdout.
 * @param message The message to log.
 */
void logMessage(const string &message)
{
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    printf("[%lu ms] %s\n", timestamp, message.c_str());
}

/**
 * @brief Sends a string message using LoRa with addresses and packet ID.
 * @param outgoing The string to send.
 */
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
    logMessage(messageToLog);
}

/**
 * @brief Sends data in chunks if larger than the max packet size.
 * @param data Pointer to the data buffer.
 * @param length Size of the data buffer.
 */
void sendLargePacket(const uint8_t* data, size_t length)
{
    size_t offset = 0;
    while (offset < length)
    {
        size_t chunkSize = (length - offset) < MAX_PKT_SIZE ? (length - offset) : MAX_PKT_SIZE;
        LoRa.beginPacket();
        LoRa.write(&data[offset], chunkSize);
        LoRa.endPacket();
        offset += chunkSize;
        sleep_ms(100); 
    }
}

/**
 * @brief Sends a string message using a universal approach to handle overhead and chunking.
 * @param outgoing The string to send.
 */
void sendMessageUniversal(const std::string& outgoing)
{
    const size_t n = outgoing.length();
    char sendBuf[n + 1];
    strcpy(sendBuf, outgoing.c_str());

    logMessage("Sat (0x" + std::to_string(localAddress) + ") to ground(" +
               std::to_string(destination) + "): " +
               std::string(sendBuf) + " [" + std::to_string(n) + "]");

    // Calculate max payload that fits LoRa + overhead
    // Overhead: destination + localAddress + msgCount + 1-byte length
    // So for example, if you set total buffer to 255, overhead is 4 bytes:
    // => actual usable payload = 255 - 4
    const size_t MAX_OVERHEAD = 4;
    const size_t MAX_TOTAL_SIZE = 255; // Adjust to your LoRa library limits
    const size_t MAX_PAYLOAD_SIZE = (MAX_TOTAL_SIZE > MAX_OVERHEAD)
                                      ? (MAX_TOTAL_SIZE - MAX_OVERHEAD)
                                      : 0;

    size_t offset = 0;
    while (offset < n)
    {
        // Calculate chunk size
        size_t chunkSize = (n - offset < MAX_PAYLOAD_SIZE)
                              ? (n - offset)
                              : MAX_PAYLOAD_SIZE;

        LoRa.beginPacket();
        LoRa.write(destination);         // add destination address
        LoRa.write(localAddress);        // add sender address
        LoRa.write(msgCount);           // add message ID
        LoRa.write(chunkSize);          // add payload length
        LoRa.write((uint8_t*)&sendBuf[offset], chunkSize); // add payload
        LoRa.endPacket(false);

        offset += chunkSize;
        msgCount++;

        sleep_ms(100);
    }
}

/**
 * @brief Callback for receiving LoRa packets; validates and forwards commands.
 * @param packetSize The size of the incoming packet.
 */
void onReceive(int packetSize)
{
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    logMessage("onReceive: packetSize = " + std::to_string(packetSize));
    if (packetSize == 0)
        return; 
    int recipient = LoRa.read();          // recipient address
    uint8_t sender = LoRa.read();         // sender address
    uint8_t incomingMsgId = LoRa.read();  // incoming msg ID
    uint8_t incomingLength = LoRa.read(); // incoming msg length

    string incoming = "";

    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }

    if (incomingLength != incoming.length() + 1)
    { 
        printf("error: message length does not match length\n");
        return; 
    }

    if (recipient != localAddress && recipient != 0xFF)
    {
        printf("This message is not for me.\n");
        return; 
    }

    logMessage("Received message of size " + std::to_string(incomingLength) + " with ID " + std::to_string(incomingMsgId) + " from: 0x" + std::to_string(sender));
    logMessage("Message: " + incoming);
    logMessage("RSSI: " + std::to_string(LoRa.packetRssi()) + " Snr: " + std::to_string(LoRa.packetSnr()));

    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    handleCommandMessage(incoming);

    lastReceiveTime = to_ms_since_boot(get_absolute_time());
}