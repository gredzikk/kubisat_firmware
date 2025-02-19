#include "unity.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

extern void test_example(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    stdio_init_all(); // Initialize stdio
    uart_init(uart0, 115200);
    gpio_set_function(0, UART_FUNCSEL_NUM(DEBUG_UART_PORT, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(DEBUG_UART_PORT, 1));

    uart_puts(uart0, "Starting tests...\n");
    UNITY_BEGIN();

    uart_puts(uart0, "Running test_example...\n");
    RUN_TEST(test_example);
    uart_puts(uart0, "test_example complete.\n");

    uart_puts(uart0, "Tests complete.\n");
    return UNITY_END();
}