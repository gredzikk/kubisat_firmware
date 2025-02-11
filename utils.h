#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pin_config.h"

void uartPrint(const std::string& msg, uart_inst_t* uart = DEBUG_UART_PORT, bool logToCore1 = false);

#endif