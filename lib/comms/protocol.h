// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <cstring>
#include "utils.h"
#include "time.h"
#include "build_number.h"
#include "LoRa/LoRa-RP2040.h"

/**
 * @def FRAME_BEGIN
 * @brief Defines the start marker for a communication frame.
 */
const std::string FRAME_BEGIN = "KBST";

/**
 * @def FRAME_END
 * @brief Defines the end marker for a communication frame.
 */
const std::string FRAME_END = "TSBK";

/**
 * @def DELIMITER
 * @brief Defines the delimiter used to separate fields within a communication frame.
 */
const char DELIMITER = ';';


/**
 * @enum ErrorCode
 * @brief Standard error codes for command responses
 */
enum class ErrorCode {
    PARAM_UNNECESSARY,    // Parameter provided but not needed
    PARAM_REQUIRED,       // Required parameter missing
    PARAM_INVALID,        // Parameter has invalid format or value
    INVALID_OPERATION,    // Operation not allowed for this command
    NOT_ALLOWED,          // Operation not permitted
    INVALID_FORMAT,       // Input format is incorrect
    INVALID_VALUE,        // Value is outside expected range
    FAIL_TO_SET,          // Failed to set provided value
    INTERNAL_FAIL_TO_READ,// Failed to read from device in remote
    UNKNOWN_ERROR         // Generic error
};


/**
 * @enum OperationType
 * @brief Represents the type of operation being performed.
 */
enum class OperationType {
    /** @brief Get data. */
    GET,
    /** @brief Set data. */
    SET,
    /** @brief Set command result. */
    RES,
    /** @brief Get command value. */
    VAL,
    /** @brief Sequence element response */
    SEQ,
    /** @brief Error occurred during command execution. */
    ERR,

};



/**
 * @enum CommandAccessLevel
 * @brief Represents the access level required to execute a command.
 */
enum class CommandAccessLevel {
    /** @brief No access allowed. */
    NONE,
    /** @brief Read-only access. */
    READ_ONLY,
    /** @brief Write-only access. */
    WRITE_ONLY,
    /** @brief Read and write access. */
    READ_WRITE
};



/**
 * @enum ValueUnit
 * @brief Represents the unit of measurement for a payload value.
 */
enum class ValueUnit {
    /** @brief Unit is undefined. */
    UNDEFINED,
    /** @brief Unit is seconds. */
    SECOND,
    /** @brief Unit is volts. */
    VOLT,
    /** @brief Unit is boolean. */
    BOOL,
    /** @brief Unit is date and time. */
    DATETIME,
    /** @brief Unit is text. */
    TEXT,
    /** @brief Unit is milliamperes. */
    MILIAMP,
};



/**
 * @enum ExceptionType
 * @brief Represents the type of exception that occurred during command execution.
 */
enum class ExceptionType {
    /** @brief No exception. */
    NONE,
    /** @brief Operation not allowed. */
    NOT_ALLOWED,
    /** @brief Invalid parameter provided. */
    INVALID_PARAM,
    /** @brief Invalid operation requested. */
    INVALID_OPERATION,
    /** @brief Parameter is unnecessary for the operation. */
    PARAM_UNECESSARY
};



/**
 * @enum Interface
 * @brief Represents the communication interface being used.
 */
enum class Interface {
    /** @brief UART interface. */
    UART,
    /** @brief LoRa interface. */
    LORA
};


/**
 * @struct Frame
 * @brief Represents a communication frame used for data exchange.
 *
 * @details This structure encapsulates the different components of a communication frame,
 *          including the header, direction, operation type, group ID, command ID, payload value, unit, and footer.
 *          It is used for both encoding and decoding messages.
 *
 * @note The `header` and `footer` fields are used to mark the beginning and end of the frame, respectively.
 * @note The `direction` field indicates the direction of the communication (0 = ground->sat, 1 = sat->ground).
 * @note The `operationType` field specifies the type of operation being performed (e.g., GET, SET, ANS, ERR, INF).
 * @note The `group` and `command` fields identify the specific command being executed.
 * @note The `value` field contains the payload data.
 * @note The `unit` field specifies the unit of measurement for the payload data.
 *
 * Example Usage:
 * @code
 * // Creating a Frame instance
 * Frame myFrame;
 * myFrame.header = FRAME_BEGIN;
 * myFrame.direction = 1;
 * myFrame.operationType = OperationType::ANS;
 * myFrame.group = 2;
 * myFrame.command = 5;
 * myFrame.value = "25.5";
 * myFrame.unit = "VOLT";
 * myFrame.footer = FRAME_END;
 *
 * // Encoding the Frame to a string
 * std::string encodedFrame = frame_encode(myFrame);
 * @endcode
 *
 * Example Instances:
 * @code
 * // Example of a GET command
 * Frame getCommand;
 * getCommand.header = FRAME_BEGIN;
 * getCommand.direction = 0;
 * getCommand.operationType = OperationType::GET;
 * getCommand.group = 1;
 * getCommand.command = 10;
 * getCommand.value = "";
 * getCommand.unit = "";
 * getCommand.footer = FRAME_END;
 *
 * // Example of an ANSWER command
 * Frame answerCommand;
 * answerCommand.header = FRAME_BEGIN;
 * answerCommand.direction = 1;
 * answerCommand.operationType = OperationType::ANS;
 * answerCommand.group = 1;
 * answerCommand.command = 10;
 * answerCommand.value = "OK";
 * answerCommand.unit = "";
 * answerCommand.footer = FRAME_END;
 * @endcode
 *
 * Example of Encoded Frames:
 * @code
 * // Encoded GET command example:
 * // KBST;0;GET;1;10;;TSBK
 *
 * // Encoded SET command example:
 * // KBST;0;SET;2;5;25.5;VOLT;TSBK
 *
 * // Encoded ANSWER command example:
 * // KBST;1;ANS;1;10;OK;;TSBK
 *
 * // Encoded ERROR command example:
 * // KBST;1;ERR;3;1;Invalid Parameter;;TSBK
 *
 * // Encoded INFO command example:
 * // KBST;1;INF;4;2;System Booted;;TSBK
 * @endcode
 */
struct Frame {
    std::string header;             // Start marker
    uint8_t direction;              // 0 = ground->sat, 1 = sat->ground
    OperationType operationType;
    uint8_t group;                  // Group ID
    uint8_t command;                // Command ID within group
    std::string value;              // Payload value
    std::string unit;               // Payload unit
    std::string footer;             // End marker
};

std::string exception_type_to_string(ExceptionType type);
std::string error_code_to_string(ErrorCode code);
std::string operation_type_to_string(OperationType type);
OperationType string_to_operation_type(const std::string& str);
std::vector<uint8_t> hex_string_to_bytes(const std::string& hexString);
std::string value_unit_type_to_string(ValueUnit unit);

#endif