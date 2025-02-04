#include <string>
#include <vector>
#include <cstdint>

// Outgoing/message global variables
extern std::string outgoing;
extern uint8_t msgCount;
extern long lastSendTime;
extern unsigned long interval;
extern long lastReceiveTime;
extern long lastPrintTime;

bool initializeRadio();
void logMessage(const std::string &message);
void sendMessage(std::string outgoing);
void sendLargePacket(const uint8_t* data, size_t length);
void onReceive(int packetSize);