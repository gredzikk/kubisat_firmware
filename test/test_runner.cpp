// test/test_runner.cpp
#include "includes.h"
#include "unity.h"

extern void test_set_time_success(void);
extern void test_set_time_invalid_time(void);
extern void test_set_time_bcd_conversion(void);
extern void test_get_time_success(void);
extern void test_timezone_offset(void);
extern void test_read_power_manager_ids(void);
extern void test_get_voltage_battery(void);
extern void test_get_voltage_5v(void);
extern void test_frame_encode_decode(void);
extern void test_frame_encode_decode_no_unit(void);
extern void test_frame_decode_invalid_header(void);
extern void test_frame_decode_missing_footer(void);
extern void test_frame_build_val(void);
extern void test_frame_build_err(void);
extern void test_frame_build_res(void);
extern void test_frame_build_seq(void);

int main(void) {
    stdio_init_all();
    i2c_init(MAIN_I2C_PORT, 400 * 1000);
    gpio_set_function(MAIN_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAIN_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MAIN_I2C_SCL_PIN);
    gpio_pull_up(MAIN_I2C_SDA_PIN);
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    PowerManager::get_instance().initialize();

    UNITY_BEGIN();
    
    RUN_TEST(test_set_time_success);
    RUN_TEST(test_set_time_invalid_time);
    RUN_TEST(test_set_time_bcd_conversion);
    RUN_TEST(test_get_time_success);
    RUN_TEST(test_timezone_offset);
    RUN_TEST(test_read_power_manager_ids);
    RUN_TEST(test_get_voltage_battery);
    RUN_TEST(test_get_voltage_5v);
    RUN_TEST(test_frame_encode_decode);
    RUN_TEST(test_frame_encode_decode_no_unit);
    RUN_TEST(test_frame_decode_invalid_header);
    RUN_TEST(test_frame_decode_missing_footer);
    RUN_TEST(test_frame_build_val);
    RUN_TEST(test_frame_build_err);
    RUN_TEST(test_frame_build_res);
    RUN_TEST(test_frame_build_seq);
    
    return UNITY_END();
}