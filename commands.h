#include <string>
#include <map>
#include <functional>
#include "pico/stdlib.h"

class CommandMessage {
public:
    std::string command;
    std::string parameter;
    
    CommandMessage(const std::string& message) {
        size_t spacePos = message.find(' ');
        if(spacePos != std::string::npos) {
            command = message.substr(0, spacePos);
            parameter = message.substr(spacePos + 1);
        } else {
            command = message;
        }
    }
};

// Type alias for command handler functions
using CommandHandler = std::function<void(const CommandMessage&)>;

// Command registry mapping command names to their handlers
extern std::map<std::string, CommandHandler> commandRegistry;

void sendMessage(std::string outgoing);
void handleCommandMessage(const std::string& message);
// Existing functions (you can keep them or call them from the lambdas below)
void handleGetTime();
void handleGetVoltageBattery();
void handleGetVoltage5V();
void handleGetCurrentChargeUSB();
void handleGetCurrentChargeSolar();
void handleGetCurrentChargeTotal();
void handleGetCurrentDraw();
void handleGetGPSPowerStatus();
void handleSetGPSPowerStatus(const std::string& param);
void handleEnableGPSTransparentMode(const std::string& timeout);

void handleUnknownCommand(const CommandMessage& commandMsg);