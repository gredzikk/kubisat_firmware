// test_runner.cpp
#include "unity.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

// External test function declarations
extern void test_frame_encode_basic(void);
extern void test_frame_decode_basic(void);
extern void test_frame_decode_invalid_header(void);
extern void test_frame_build_success(void);
extern void test_frame_build_error(void);
extern void test_frame_build_info(void);
extern void test_operation_type_conversion(void);
extern void test_value_unit_type_conversion(void);
extern void test_exception_type_conversion(void);
extern void test_hex_string_conversion(void);

int main(void) {
    stdio_init_all();
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    UNITY_BEGIN();
    uart_puts(uart0, "begin unity tests\n");
    
    // Frame codec tests
    uart_puts(uart0, "begin frame codec tests\n");
    RUN_TEST(test_frame_encode_basic);
    RUN_TEST(test_frame_decode_basic);
    RUN_TEST(test_frame_decode_invalid_header);
    uart_puts(uart0, "end frame codec tests\n");
 
    // Frame build tests
    uart_puts(uart0, "begin frame build tests\n");
    RUN_TEST(test_frame_build_success);
    RUN_TEST(test_frame_build_error);
    RUN_TEST(test_frame_build_info);
    uart_puts(uart0, "end frame build tests\n");
    
    // Converter tests
    uart_puts(uart0, "begin converter tests\n");
    RUN_TEST(test_operation_type_conversion);
    RUN_TEST(test_value_unit_type_conversion);
    RUN_TEST(test_exception_type_conversion);
    RUN_TEST(test_hex_string_conversion);
    uart_puts(uart0, "end converter tests\n");

    return UNITY_END();
}