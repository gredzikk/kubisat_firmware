#include "communication.h"


/**
 * @file utils_converters.cpp
 * @brief Implements utility functions for converting between different data types.
 * @defgroup UtilsConverters Utility Converters
 */

/**
 * @brief Converts an ExceptionType to a string.
 * @param type The ExceptionType to convert.
 * @return The string representation of the ExceptionType.
 * @ingroup UtilsConverters
 */
std::string exception_type_to_string(ExceptionType type) {
    switch (type) {
        case ExceptionType::NOT_ALLOWED:       return "NOT ALLOWED";
        case ExceptionType::INVALID_PARAM:     return "INVALID PARAM";
        case ExceptionType::INVALID_OPERATION: return "INVALID OPERATION";
        case ExceptionType::PARAM_UNECESSARY:  return "PARAM UNECESSARY";
        case ExceptionType::NONE:              return "NONE";
        default:                               return "UNKNOWN EXCEPTION";
    }
}


/**
 * @brief Converts a ValueUnit to a string.
 * @param unit The ValueUnit to convert.
 * @return The string representation of the ValueUnit.
 * @ingroup UtilsConverters
 */
std::string value_unit_type_to_string(ValueUnit unit) {
    switch (unit) {
        case ValueUnit::UNDEFINED:  return "";
        case ValueUnit::SECOND:     return "s";
        case ValueUnit::VOLT:       return "V";
        case ValueUnit::BOOL:       return "";
        case ValueUnit::DATETIME:   return "";
        case ValueUnit::TEXT:       return "";
        case ValueUnit::MILIAMP:    return "mA";
        default: return "";
    }
}


/**
 * @brief Converts an OperationType to a string.
 * @param type The OperationType to convert.
 * @return The string representation of the OperationType.
 * @ingroup UtilsConverters
 */
std::string operation_type_to_string(OperationType type) {
    switch (type) {
        case OperationType::GET: return "GET";
        case OperationType::SET: return "SET";
        case OperationType::VAL: return "VAL";
        case OperationType::ERR: return "ERR";
        case OperationType::RES: return "RES";
        case OperationType::SEQ: return "SEQ";
        default: return "UNKNOWN";
    }
}


/**
 * @brief Converts a string to an OperationType.
 * @param str The string to convert.
 * @return The OperationType corresponding to the string. Defaults to GET if the string is not recognized.
 * @ingroup UtilsConverters
 */
OperationType string_to_operation_type(const std::string& str) {
    if (str == "GET") return OperationType::GET;
    if (str == "SET") return OperationType::SET;
    if (str == "VAL") return OperationType::VAL;
    if (str == "ERR") return OperationType::ERR;
    if (str == "RES") return OperationType::RES;
    if (str == "SEQ") return OperationType::SEQ;
    return OperationType::GET; // Default to GET
}

/**
 * @brief Converts an ErrorCode to its string representation
 * @param code The error code
 * @return String representation of the error code
 * @ingroup UtilsConverters
 */
std::string error_code_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::PARAM_UNNECESSARY:      return "PARAM_UNNECESSARY";
        case ErrorCode::PARAM_REQUIRED:         return "PARAM_REQUIRED";
        case ErrorCode::PARAM_INVALID:          return "PARAM_INVALID";
        case ErrorCode::INVALID_OPERATION:      return "INVALID_OPERATION";
        case ErrorCode::NOT_ALLOWED:            return "NOT_ALLOWED";
        case ErrorCode::INVALID_FORMAT:         return "INVALID_FORMAT";
        case ErrorCode::INVALID_VALUE:          return "INVALID_VALUE";
        case ErrorCode::FAIL_TO_SET:            return "FAIL_TO_SET";
        case ErrorCode::INTERNAL_FAIL_TO_READ:  return "INTERNAL_FAIL_TO_READ";
        default:                                return "UNKNOWN_ERROR";
    }
}


/**
 * @brief Converts a hex string to a vector of bytes.
 * @param hexString The hex string to convert.
 * @return A vector of bytes representing the hex string.
 * @ingroup UtilsConverters
 */
std::vector<uint8_t> hex_string_to_bytes(const std::string& hexString) {
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

/** @} */ 