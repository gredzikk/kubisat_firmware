#include "unity.h"
#include "PowerManager.h"
#include <string>

void test_read_power_manager_ids(void) {
    PowerManager& powerManager = PowerManager::get_instance();
    std::string device_ids = powerManager.read_device_ids();
    TEST_ASSERT(device_ids == "MAN 0x5449 - DIE 0x3220");
}

void test_get_voltage_battery(void) {
    PowerManager& powerManager = PowerManager::get_instance();
    float voltage = powerManager.get_voltage_battery();
    TEST_ASSERT_FLOAT_WITHIN(0.5, 3.3, voltage);
}

void test_get_voltage_5v(void) {
    PowerManager& powerManager = PowerManager::get_instance();
    float voltage = powerManager.get_voltage_5v();
    TEST_ASSERT_FLOAT_WITHIN(0.5, 5.0, voltage);
}