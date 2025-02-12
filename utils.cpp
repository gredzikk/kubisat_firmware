#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "storage.h"

#define LOG_FILENAME "log.txt"

static mutex_t uart_mutex;

void uartPrint(const std::string& msg, bool logToFile, uart_inst_t* uart) {
    static bool mutex_inited = false;
    if (!mutex_inited) {
        mutex_init(&uart_mutex);
        mutex_inited = true;
    }

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint core_num = get_core_num(); // Get the current core number
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - Core " + std::to_string(core_num) + ": " + msg + "\r\n";

    if (logToFile) {

        // Send the entire message to Core 1
        const char* cstr = msgToSend.c_str();
        //writeToFile(LOG_FILENAME, cstr);
        for (size_t i = 0; i < msgToSend.length(); ++i) {
            multicore_fifo_push_blocking(cstr[i]);
        }
        multicore_fifo_push_blocking(0); // Null terminator to signal end of message
    }

    mutex_enter_blocking(&uart_mutex);
    uart_puts(uart, msgToSend.c_str());
    mutex_exit(&uart_mutex);
}

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