#include "utils.h"

void uartPrint(const std::string msg, uart_inst_t* uart)
{
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - " + msg;
    uart_puts(uart, msgToSend.c_str());
    uart_puts(uart, "\r\n");
}