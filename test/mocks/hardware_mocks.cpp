// test/mocks/hardware_mocks.cpp
#include "hardware_mocks.h"
#include <cstring>

// UART mocks
bool mock_uart_enabled = false;
std::vector<std::string> uart_output_buffer;

void mock_uart_puts(uart_inst_t* uart, const char* str) {
    if (mock_uart_enabled) {
        uart_output_buffer.push_back(std::string(str));
    } else {
        uart_puts(uart, str);
    }
}

void mock_uart_init(uart_inst_t* uart, uint baudrate) {
    if (!mock_uart_enabled) {
        uart_init(uart, baudrate);
    }
}

// SPI mocks
bool mock_spi_enabled = false;
std::vector<uint8_t> spi_output_buffer;

void mock_spi_write_blocking(spi_inst_t* spi, const uint8_t* src, size_t len) {
    if (mock_spi_enabled) {
        spi_output_buffer.insert(spi_output_buffer.end(), src, src + len);
    } else {
        spi_write_blocking(spi, src, len);
    }
}

int mock_spi_read_blocking(spi_inst_t* spi, uint8_t tx_data, uint8_t* dst, size_t len) {
    if (mock_spi_enabled) {
        // Mock implementation that fills dst with test data
        memset(dst, tx_data, len);
        return len;
    } else {
        return spi_read_blocking(spi, tx_data, dst, len);
    }
}