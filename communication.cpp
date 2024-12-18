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

    if (!LoRa.begin(500E6))
    {
        logMessage("LoRa init failed. Check your connections.");
        return false;
            
    }

    logMessage(" init succeeded.");
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
    logMessage("Sat to ground: " + string(send) + " [" + std::to_string(n) + "]");
    LoRa.beginPacket();       // start packet
    LoRa.write(destination);  // add destination address
    LoRa.write(localAddress); // add sender address
    LoRa.write(msgCount);     // add message ID
    LoRa.write(n + 1);        // add payload length
    LoRa.print(send);         // add payload
    LoRa.endPacket();         // finish packet and send it
    msgCount++;               // increment message ID
}

void onReceive(int packetSize)
{
    if (packetSize == 0)
        return; // if there's no packet, return
    // read packet header uint8_ts:
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
    { // check length for error
        printf("error: message length does not match length\n");
        return; // skip rest of function
    }

    // if the recipient isn't this device or broadcast,
    if (recipient != localAddress && recipient != 0xFF)
    {
        printf("This message is not for me.\n");
        return; // skip rest of function
    }

    logMessage("Received from: 0x" + std::to_string(sender));
    logMessage("Sent to: 0x" + std::to_string(recipient));
    logMessage("Message ID: " + std::to_string(incomingMsgId));
    logMessage("Message length: " + std::to_string(incomingLength));
    logMessage("Message: " + incoming);
    logMessage("RSSI: " + std::to_string(LoRa.packetRssi()));
    logMessage("Snr: " + std::to_string(LoRa.packetSnr()));

    sleep_ms(150);
    handleCommand(incoming);

    // Update last receive time
    lastReceiveTime = to_ms_since_boot(get_absolute_time());
}