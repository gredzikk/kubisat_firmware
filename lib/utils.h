#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pin_config.h"
#include <vector>


/**
 * @file utils.h
 * @brief Utility functions and definitions for the Kubisat firmware
 * @details Contains UART logging, color definitions, and CRC calculations
 */


/** @brief ANSI escape codes for terminal color output */
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_RESET   "\033[0m"


/**
 * @brief Verbosity levels for logging system
 */
enum class VerbosityLevel {
    SILENT = 0,  /**< No output */
    ERROR = 1,   /**< Only critical errors */
    WARNING = 2, /**< Warnings and errors */
    INFO = 3,    /**< Normal operation information */
    DEBUG = 4    /**< Detailed debug information */
};


/**
 * @brief Global verbosity level setting for UART output
 * @details Controls which messages are displayed:
 *          - SILENT (0): No output
 *          - ERROR (1): Only errors
 *          - WARNING (2): Warnings and errors
 *          - INFO (3): Normal operation information
 *          - DEBUG (4): Detailed debug information
 * @note Can be changed at runtime through the command interface
 * @see VerbosityLevel
 * @see handle_verbosity
 */
extern VerbosityLevel g_uart_verbosity;


/**
 * @brief Prints a message to UART with timestamp and formatting
 * @param msg The message to print
 * @param level Message verbosity level
 * @param logToFile Whether to store the message in log storage
 * @param uart The UART port to use
 */
void uart_print(const std::string& msg, 
               VerbosityLevel level = VerbosityLevel::INFO,
               bool logToFile = false, 
               uart_inst_t* uart = DEBUG_UART_PORT);



/**
 * @brief Calculates CRC16 checksum
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @return Calculated CRC16 value
 */
uint16_t crc16(const uint8_t *data, size_t length);

#endif