#include "utils.h"
#include "pico/multicore.h"

void uartPrint(const std::string& msg, uart_inst_t* uart, bool logToCore1) {
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - " + msg + "\r\n";

    if (logToCore1) {
        for (char c : msgToSend) {
            multicore_fifo_push_blocking(c);
        }
        multicore_fifo_push_blocking(0); // Null terminator to signal end of message
    }

    uart_puts(uart, msgToSend.c_str());
}