// communication.cpp
#include "LoRa-RP2040.h"
#include "commands.h"
#include "communication.h"

using std::string;

std::string outgoing;                // outgoing message
uint8_t msgCount = 0;                // count of outgoing messages
long lastSendTime = 0;                // last send time
long lastReceiveTime = 0;             // last receive time
long lastPrintTime = 0;  

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
// LoRa methods
void logMessage(const string &message)
{
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    printf("[%lu ms] %s\n", timestamp, message.c_str());
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
    LoRa.endPacket(false);         // finish packet and send it
    msgCount++;               // increment message ID
}

void onReceive(int packetSize)
{
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

    handleCommand(incoming);

    lastReceiveTime = to_ms_since_boot(get_absolute_time());
}