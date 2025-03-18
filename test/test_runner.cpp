// test/test_runner.cpp
#include "includes.h"
#include "unity.h"

extern void test_set_time_success(void);
extern void test_set_time_i2c_failure(void);
extern void test_set_time_invalid_time(void);
extern void test_set_time_bcd_conversion(void);
extern void test_get_time_success(void);
extern void test_timezone_offset(void);
extern void test_get_local_time(void);
extern void test_read_temperature(void);
extern void test_read_temperature_i2c_failure(void);

int main(void) {
    stdio_init_all();
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    UNITY_BEGIN();
    uart_puts(uart0, "begin unity tests\n");
    
    RUN_TEST(test_set_time_success);
    RUN_TEST(test_set_time_i2c_failure);
    RUN_TEST(test_set_time_invalid_time);
    RUN_TEST(test_set_time_bcd_conversion);
    
    return UNITY_END();
}
