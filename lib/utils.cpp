#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#define LOG_FILENAME "log.txt"

static mutex_t uart_mutex;


/**
 * @file utils.cpp
 * @brief Implements utility functions for the Kubisat firmware.
 * @details This file contains various utility functions, including UART printing with timestamps,
 *          and CRC16 calculation.
 */


/**
 * @brief Prints a message to the UART with a timestamp and core number.
 * @param msg The message to print.
 * @param logToFile A flag indicating whether to log the message to a file (currently not implemented).
 * @param uart The UART instance to use for printing.
 * @details Prints the given message to the specified UART, prepending it with a timestamp and the core number.
 *          Uses a mutex to ensure thread-safe access to the UART.
 */
void uart_print(const std::string& msg, bool logToFile, uart_inst_t* uart) {
    static bool mutex_inited = false;
    if (!mutex_inited) {
        mutex_init(&uart_mutex);
        mutex_inited = true;
    }

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint core_num = get_core_num();
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - Core " + std::to_string(core_num) + ": " + msg + "\r\n";

    if (logToFile) {
        logToFile = !logToFile;
    }
    mutex_enter_blocking(&uart_mutex);
    uart_puts(uart, msgToSend.c_str());
    mutex_exit(&uart_mutex);
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
