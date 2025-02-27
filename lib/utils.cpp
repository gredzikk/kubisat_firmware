#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include <vector>
#include <queue>
#include <string>
#include <array>


/**
 * @file utils.cpp
 * @brief Implementation of utility functions for the Kubisat firmware
 */


/** @brief Mutex for UART access protection */
static mutex_t uart_mutex;


/** @brief Global verbosity level setting */
VerbosityLevel g_uart_verbosity = VerbosityLevel::DEBUG;


/**
 * @brief Gets ANSI color code for verbosity level
 * @param level The verbosity level
 * @return ANSI color escape sequence
 */
std::string get_level_color(VerbosityLevel level) {
    switch (level) {
        case VerbosityLevel::ERROR:   return ANSI_RED;
        case VerbosityLevel::WARNING: return ANSI_YELLOW;
        case VerbosityLevel::INFO:    return ANSI_GREEN;
        case VerbosityLevel::DEBUG:   return ANSI_BLUE;
        case VerbosityLevel::EVENT:    return ANSI_CYAN;
        default:                      return "";
    }
}


/**
 * @brief Gets text prefix for verbosity level
 * @param level The verbosity level
 * @return Text prefix for the level
 */
std::string get_level_prefix(VerbosityLevel level) {
    switch (level) {
        case VerbosityLevel::ERROR:   return "ERROR: ";
        case VerbosityLevel::WARNING: return "WARNING: ";
        case VerbosityLevel::INFO:    return "INFO: ";
        case VerbosityLevel::DEBUG:   return "DEBUG: ";
        case VerbosityLevel::EVENT:   return "EVENT: ";
        default:                      return "";
    }
}

/**
 * @brief Prints a message to the UART with a timestamp and core number.
 * @param msg The message to print.
 * @param logToFile A flag indicating whether to log the message to a file (currently not implemented).
 * @param uart The UART instance to use for printing.
 * @details Prints the given message to the specified UART, prepending it with a timestamp and the core number.
 *          Uses a mutex to ensure thread-safe access to the UART.
 */
void uart_print(const std::string& msg, VerbosityLevel level, bool logToFile, uart_inst_t* uart) {
    if (static_cast<int>(level) > static_cast<int>(g_uart_verbosity)) {
        return;
    }

    static bool mutex_inited = false;
    if (!mutex_inited) {
        mutex_init(&uart_mutex);
        mutex_inited = true;
    }

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint core_num = get_core_num();

    std::string color = get_level_color(level);
    std::string prefix = get_level_prefix(level);
    std::string msg_to_send = "[" + std::to_string(timestamp) + "ms] - Core " + 
                           std::to_string(core_num) + ": " + 
                           color + prefix + ANSI_RESET + msg + "\r\n";

    mutex_enter_blocking(&uart_mutex);
    uart_puts(uart, msg_to_send.c_str());
    mutex_exit(&uart_mutex);
}


// Base64 encoding table
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const uint8_t* input, size_t length) {
    std::string result;
    result.reserve(((length + 2) / 3) * 4); // Reserve space for the result
    
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    // Handle the remaining bytes
    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];
        
        while (i++ < 3)
            result += '='; // Padding
    }
    
    return result;
}



uint32_t calculate_checksum(const uint8_t* data, size_t length) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}