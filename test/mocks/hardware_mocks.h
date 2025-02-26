// test/mocks/hardware_mocks.h
#ifndef HARDWARE_MOCKS_H
#define HARDWARE_MOCKS_H

#include <vector>
#include <string>

// UART mocks
extern bool mock_uart_enabled;
extern std::vector<std::string> uart_output_buffer;

void mock_uart_puts(uart_inst_t* uart, const char* str);
void mock_uart_init(uart_inst_t* uart, uint baudrate);

// SPI mocks
extern bool mock_spi_enabled;
extern std::vector<uint8_t> spi_output_buffer;

void mock_spi_write_blocking(spi_inst_t* spi, const uint8_t* src, size_t len);
int mock_spi_read_blocking(spi_inst_t* spi, uint8_t tx_data, uint8_t* dst, size_t len);

#endif // HARDWARE_MOCKS_H