#include "communication.h"

using CommandHandler = std::function<std::string(const std::string&, OperationType)>;
extern std::map<uint32_t, CommandHandler> commandHandlers;
extern volatile uint16_t eventRegister;

/**
 * @file frame.cpp
 * @brief Implements functions for encoding, decoding, building, and processing Frames.
 */

/**
 * @brief Encodes a Frame instance into a string.
 * @param frame The Frame instance to encode.
 * @return The Frame encoded as a string.
 * @details The encoded string includes the frame direction, operation type, group, command, value, and unit,
 *          all delimited by the DELIMITER character. The string is encapsulated by FRAME_BEGIN and FRAME_END.
 */
std::string frame_encode(const Frame& frame) {
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operation_type_to_string(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value;

    if (!frame.unit.empty()) {
        ss << DELIMITER << frame.unit;
    }

    return FRAME_BEGIN + DELIMITER + ss.str() + DELIMITER + FRAME_END;
}


/**
 * @brief Converts a string into a Frame instance.
 * @param data The Frame data as a string.
 * @return The Frame instance.
 * @throws std::runtime_error if the frame header is invalid.
 * @details Parses the input string, extracting the frame direction, operation type, group, command, value, and unit.
 *          If an error occurs during parsing, an error message is printed to the UART, an error frame is built and sent,
 *          and the exception is re-thrown.
 */
Frame frame_decode(const std::string& data) {
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
        frame.operationType = string_to_operation_type(token);

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
        uart_print("Frame error: " + std::string(e.what()));
        Frame errorFrame = frame_build(ExecutionResult::ERROR, 0, 0, e.what()); 
        send_frame(errorFrame);
        throw; 
    }
}


/**
 * @brief Executes a command based on the command key and the parameter.
 * @param data The Frame data in string format.
 * @details Decodes the frame data, extracts the command key, and executes the corresponding command.
 *          Sends the response frame. If an error occurs, an error frame is built and sent.
 */
void frame_process(const std::string& data, Interface interface) {
    try {
        Frame frame = frame_decode(data);
        uint32_t commandKey = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);

        Frame responseFrame = execute_command(commandKey, frame.value, frame.operationType);

        // Send response through the same interface that received the command
        if (interface == Interface::UART) {
            send_frame_uart(responseFrame);
        } else if (interface == Interface::LORA) {
            send_frame_lora(responseFrame);
        }
    } catch (const std::exception& e) {
        Frame errorFrame = frame_build(ExecutionResult::ERROR, 0, 0, e.what()); 
        // Send error through the same interface
        if (interface == Interface::UART) {
            send_frame_uart(errorFrame);
        } else if (interface == Interface::LORA) {
            send_frame_lora(errorFrame);
        }
    }
}

/**
 * @brief Builds a Frame instance.
 * @param result The execution result.
 * @param group The group ID.
 * @param command The command ID.
 * @param value The value.
 * @param unitType The value unit type.
 * @return The Frame instance.
 * @details Constructs a Frame instance based on the provided parameters. The frame direction and operation type
 *          are set based on the execution result.
 */
Frame frame_build(ExecutionResult result, uint8_t group, uint8_t command, 
                const std::string& value, const ValueUnit unitType) {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;
    
    switch (result) {
        case ExecutionResult::SUCCESS:
            frame.direction = 1;
            frame.operationType = OperationType::ANS;
            frame.value = value;
            frame.unit = value_unit_type_to_string(unitType);
            break;
            
        case ExecutionResult::ERROR:
            frame.direction = 1;
            frame.operationType = OperationType::ERR;
            frame.value = value; 
            frame.unit = value_unit_type_to_string(ValueUnit::UNDEFINED);
            break;

        case ExecutionResult::INFO:
            frame.direction = 1;
            frame.operationType = OperationType::INF;
            frame.value = value;
            frame.unit = value_unit_type_to_string(ValueUnit::UNDEFINED);
            break;
    }
    
    frame.group = group;
    frame.command = command;
    
    return frame;
}
