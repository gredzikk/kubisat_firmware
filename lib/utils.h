#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pin_config.h"

void uart_print(const std::string& msg, bool logToFile = false, uart_inst_t* uart = DEBUG_UART_PORT);
uint16_t crc16(const uint8_t *data, size_t length);

#endif