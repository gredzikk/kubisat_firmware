#include "communication.h"

/// @brief Encode Frame instance into a string
/// @param frame  Frame instance to encode
/// @return Frame encoded as a string
std::string encodeFrame(const Frame& frame) {
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operationTypeToString(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value;

    if (!frame.unit.empty()) {
        ss << DELIMITER << frame.unit;
    }

    return FRAME_BEGIN + DELIMITER + ss.str() + DELIMITER + FRAME_END;
}

/// @brief Convert a string into a Frame instance
/// @param data Frame data as a string
/// @return Frame instance
Frame decodeFrame(const std::string& data) {
    try {
        Frame frame;
        std::stringstream ss(data);
        std::string token;

        std::getline(ss, token, DELIMITER);
        if (token != FRAME_BEGIN)
            throw std::runtime_error("Invalid frame header");
        frame.header = token;

        std::string decodedFrameData;
        while (std::getline(ss, token, DELIMITER)) {
            if (token == FRAME_END) break; 
            decodedFrameData += token + DELIMITER;
        }
        if (!decodedFrameData.empty()) {
            decodedFrameData.pop_back();
        }

        std::stringstream frameDataStream(decodedFrameData);

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
        Frame errorFrame = buildFrame(ExecutionResult::ERROR, 0, 0, e.what()); 
        sendFrame(errorFrame);
        throw; 
    }
}


using CommandHandler = std::function<std::string(const std::string&, OperationType)>;
extern std::map<uint32_t, CommandHandler> commandHandlers;

/// @brief Execute the command based on the command key and the parameter
/// @param data Frame data in string format
void processFrameData(const std::string& data) {
    try {
        Frame frame = decodeFrame(data);
        uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);

        Frame responseFrame = executeCommand(commandKey, frame.value, frame.operationType);

        sendFrame(responseFrame);
    } catch (const std::exception& e) {
        Frame errorFrame = buildFrame(ExecutionResult::ERROR, 0, 0, e.what()); 
        sendFrame(errorFrame);
    }
}


extern volatile uint16_t eventRegister;

/// @brief Send event register value to the ground station
/// @note This function is called in the main loop
void sendEventRegister() {
    std::stringstream ss;
    ss << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(eventRegister);
    std::string eventValue = ss.str();

    Frame eventFrame = buildFrame(
        ExecutionResult::SUCCESS, // Result: success as this is a normal status update
        8,                        // Group ID: 8 (EVENTS)
        0,                        // Command ID: 0 (EVENT_REGISTER)
        eventValue                // Value: event register value
    );

    sendFrame(eventFrame);
}

Frame buildFrame(ExecutionResult result, uint8_t group, uint8_t command, 
                const std::string& value, const ValueUnit unitType) {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;
    
    switch (result) {
        case ExecutionResult::SUCCESS:
            frame.direction = 1;
            frame.operationType = OperationType::ANS;
            frame.value = value;
            frame.unit = valueUnitTypeToString(unitType);
            break;
            
        case ExecutionResult::ERROR:
            frame.direction = 1;
            frame.operationType = OperationType::ERR;
            frame.value = value; 
            frame.unit = valueUnitTypeToString(ValueUnit::UNDEFINED);
            break;

        case ExecutionResult::INFO:
            frame.direction = 1;
            frame.operationType = OperationType::INF;
            frame.value = value;
            frame.unit = valueUnitTypeToString(ValueUnit::UNDEFINED);
            break;
    }
    
    frame.group = group;
    frame.command = command;
    
    return frame;
}
