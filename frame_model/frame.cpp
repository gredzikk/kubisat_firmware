#include "communication.h"

// Function to convert OperationType to string
std::string operationTypeToString(OperationType type) {
    switch (type) {
        case OperationType::GET: return "GET";
        case OperationType::SET: return "SET";
        case OperationType::ANS: return "ANS";
        default: return "UNKNOWN";
    }
}

// Function to convert string to OperationType
OperationType stringToOperationType(const std::string& str) {
    if (str == "GET") return OperationType::GET;
    if (str == "SET") return OperationType::SET;
    if (str == "ANS") return OperationType::ANS;
    return OperationType::GET; // Default to GET
}

Frame buildFrame(uint8_t direction, OperationType operationType, uint8_t group, uint8_t command, const std::string& value, const std::string& unit) {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.direction = direction;
    frame.operationType = operationType;
    frame.group = group;
    frame.command = command;
    frame.value = value;
    frame.unit = unit;
    frame.footer = FRAME_END;
    return frame;
}

Frame buildErrorFrame(const std::string& errorMessage) {
    return buildFrame(1, OperationType::ERR, 0, 0, errorMessage, "");
}

// Encode a frame into a string
std::string encodeFrame(const Frame& frame) {
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operationTypeToString(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value << DELIMITER
       << frame.unit;

    return FRAME_BEGIN + DELIMITER + ss.str() + DELIMITER + FRAME_END;
}

// Decode a string into a Frame. Throws std::runtime_error if frame is bad.
Frame decodeFrame(const std::string& data) {
    try {
        Frame frame;
        std::stringstream ss(data);
        std::string token;

        std::getline(ss, token, DELIMITER);
        if (token != FRAME_BEGIN)
            throw std::runtime_error("Invalid frame header");
        frame.header = token;

        std::string frameDataWithoutCrc;
        while (std::getline(ss, token, DELIMITER)) {
            if (token == FRAME_END) break; // Stop at the footer
            frameDataWithoutCrc += token + DELIMITER;
        }
        if (!frameDataWithoutCrc.empty()) {
            frameDataWithoutCrc.pop_back();
        }

        std::stringstream frameDataStream(frameDataWithoutCrc);

        std::getline(frameDataStream, token, DELIMITER);
        frame.direction = std::stoi(token);

        std::getline(frameDataStream, token, DELIMITER);
        frame.operationType = stringToOperationType(token);

        std::getline(frameDataStream, token, DELIMITER);
        frame.group = std::stoi(token);

        std::getline(frameDataStream, token, DELIMITER);
        frame.command = std::stoi(token);

        std::getline(frameDataStream, token, DELIMITER);
        frame.value = token;

        std::getline(frameDataStream, token, DELIMITER);
        frame.unit = token;

        return frame;
    } catch (const std::exception& e) {
        uartPrint("Frame error: " + std::string(e.what()));
        Frame errorFrame = buildErrorFrame(e.what());
        sendFrame(errorFrame);
        throw; // Re-throw the exception so that the calling function knows that an error occurred.
    }
}

Frame buildResponseFrame(const Frame& requestFrame, const std::string& value) {
    Frame responseFrame;
    responseFrame.header = FRAME_BEGIN;
    responseFrame.direction = (requestFrame.direction == 0) ? 1 : 0; // Reverse direction
    responseFrame.operationType = OperationType::ANS; // ANS
    responseFrame.group = requestFrame.group;
    responseFrame.command = requestFrame.command;
    responseFrame.value = value;

    // Find the unit from getGroups
    std::vector<Group> groups = getGroups();
    for (const auto& group : groups) {
        if (group.Id == requestFrame.group) {
            for (const auto& command : group.Commands) {
                if (command.Id == requestFrame.command) {
                    // Assign the unit to the response frame
                    switch (command.Unit) {
                        case ValueUnit::VOLT:
                            responseFrame.unit = "V";
                            break;
                        case ValueUnit::BOOL:
                            responseFrame.unit = "";
                            break;
                        case ValueUnit::DATETIME:
                            responseFrame.unit = "";
                            break;
                        case ValueUnit::SECOND:
                            responseFrame.unit = "s";
                            break;
                        case ValueUnit::MILIAMP:
                            responseFrame.unit = "mA";
                            break;
                        default:
                            responseFrame.unit = "";
                            break;
                    }
                    return responseFrame;
                }
            }
        }
    }

    responseFrame.unit = "";
    return responseFrame;
}



// Command handler function type
using CommandHandler = std::function<std::string(const std::string&, OperationType)>;

// Declare command map
extern std::map<uint32_t, CommandHandler> commandHandlers;

// Once a frame is decoded, call the command handler to execute it.
void handleCommandFrame(const Frame& frame) {
    uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);
    auto it = commandHandlers.find(commandKey);

    if (it != commandHandlers.end()) {
        std::string param = frame.value;

        // Execute the command and get the response data
        Frame response = executeCommand(commandKey, param, frame.operationType);

        sendFrame(response);
    } else {
        uartPrint("Error: Unknown group/command combination");
    }
}

void processFrameData(const std::string& data) {
    try {
        Frame frame = decodeFrame(data);
        uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);

        // Execute the command and get the response frame
        Frame responseFrame = executeCommand(commandKey, frame.value, frame.operationType);

        // Send the response frame
        sendFrame(responseFrame);

    } catch (const std::exception& e) {
        // Handle decoding errors
        Frame errorFrame = buildErrorFrame(1, 0, 0, e.what()); // Generic error
        sendFrame(errorFrame);
    }
}

std::vector<uint8_t> hexStringToBytes(const std::string& hexString) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::string byteString = hexString.substr(i, 2);
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << byteString;
        ss >> byte;
        bytes.push_back(static_cast<uint8_t>(byte));
    }
    return bytes;
}



// Function to send the event register value via radio
extern volatile uint16_t eventRegister;
void sendEventRegister() {
    // Convert the event register value to a string
    std::stringstream ss;
    ss << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(eventRegister);
    std::string eventValue = ss.str();

    // Build the frame
    Frame eventFrame = buildFrame(
        1,                  // Direction: sat->ground
        OperationType::ANS, // Operation Type: ANS
        8,                  // Group ID: 8 (EVENTS)
        0,                  // Command ID: 0 (EVENT_REGISTER)
        eventValue,         // Value: event register value
        ""                  // Unit: None
    );

    // Send the frame
    sendFrame(eventFrame);
}

Frame buildSuccessFrame(uint8_t direction, uint8_t group, uint8_t command, const std::string& value, const std::string& unit) {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.direction = direction;
    frame.operationType = OperationType::ANS;
    frame.group = group;
    frame.command = command;
    frame.value = value;
    frame.unit = unit;
    frame.footer = FRAME_END;
    return frame;
}

Frame buildErrorFrame(uint8_t direction, uint8_t group, uint8_t command, const std::string& errorMessage) {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.direction = direction;
    frame.operationType = OperationType::ERR;
    frame.group = group;
    frame.command = command;
    frame.value = errorMessage;
    frame.unit = "";
    frame.footer = FRAME_END;
    return frame;
}