// test/test_runner.cpp
#include "includes.h"
#include "unity.h"

// External test function declarations
// Pure software tests (no hardware dependencies)
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

// Command handler tests
extern void test_command_handler_get_operation(void);
extern void test_command_handler_set_operation(void);
extern void test_command_handler_invalid_operation(void);

// Error code tests
extern void test_error_code_conversion(void);

int main(void) {
    stdio_init_all();
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    UNITY_BEGIN();
    uart_puts(uart0, "begin unity tests\n");
    
    // Frame codec tests (pure software)
    uart_puts(uart0, "begin frame codec tests\n");
    RUN_TEST(test_frame_encode_basic);
    RUN_TEST(test_frame_decode_basic);
    RUN_TEST(test_frame_decode_invalid_header);
    uart_puts(uart0, "end frame codec tests\n");
 
    // Frame build tests (pure software)
    uart_puts(uart0, "begin frame build tests\n");
    RUN_TEST(test_frame_build_success);
    RUN_TEST(test_frame_build_error);
    RUN_TEST(test_frame_build_info);
    uart_puts(uart0, "end frame build tests\n");
    
    // Converter tests (pure software)
    uart_puts(uart0, "begin converter tests\n");
    RUN_TEST(test_operation_type_conversion);
    RUN_TEST(test_value_unit_type_conversion);
    RUN_TEST(test_exception_type_conversion);
    RUN_TEST(test_hex_string_conversion);
    RUN_TEST(test_error_code_conversion);
    uart_puts(uart0, "end converter tests\n");
    
    // Command handler tests (pure software)
    uart_puts(uart0, "begin command handler tests\n");
    RUN_TEST(test_command_handler_get_operation);
    RUN_TEST(test_command_handler_set_operation);
    RUN_TEST(test_command_handler_invalid_operation);
    uart_puts(uart0, "end command handler tests\n");

    return UNITY_END();
}