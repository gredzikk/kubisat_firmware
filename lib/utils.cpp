#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include <vector>
#include <queue>

/**
 * @file utils.cpp
 * @brief Implementation of utility functions for the Kubisat firmware
 */


/** @brief Mutex for UART access protection */
static mutex_t uart_mutex;


/** @brief Global verbosity level setting */
VerbosityLevel g_uart_verbosity = VerbosityLevel::INFO;


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
        mutex_init(&log_mutex);
        mutex_inited = true;
    }

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint core_num = get_core_num();

    // Create formatted message with color
    std::string color = get_level_color(level);
    std::string prefix = get_level_prefix(level);
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - Core " + 
                           std::to_string(core_num) + ": " + 
                           color + prefix + msg + ANSI_RESET + "\r\n";

    // Print to UART
    mutex_enter_blocking(&uart_mutex);
    uart_puts(uart, msgToSend.c_str());
    mutex_exit(&uart_mutex);

    // Store in log buffer if requested
    if (logToFile) {
        mutex_enter_blocking(&log_mutex);
        LogMessage logMsg = {
            .timestamp = timestamp,
            .core_num = core_num,
            .level = level,
            .message = msg
        };
        pending_logs.push(logMsg);
        
        // If buffer is full, trigger storage flush
        if (pending_logs.size() >= MAX_LOG_BUFFER_SIZE) {
            flush_logs_to_storage();
        }
        mutex_exit(&log_mutex);
    }
}


/**
 * @brief Calculates the CRC16 checksum of the given data.
 * @param data A pointer to the data buffer.
 * @param length The length of the data in bytes.
 * @return The CRC16 checksum.
 * @details Calculates the CRC16 checksum using the standard algorithm.
 */
uint16_t crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
