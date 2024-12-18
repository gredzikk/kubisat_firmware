#include <string>

extern std::string outgoing;         // outgoing message
extern uint8_t msgCount;            // count of outgoing messages
extern long lastSendTime;           // last send time
extern long unsigned int interval;   // interval between sends
extern long lastReceiveTime;        // last receive time
extern long lastPrintTime;          // last print time

bool initializeRadio();

void logMessage(const string &message);

void sendMessage(string outgoing);

void onReceive(int packetSize);
