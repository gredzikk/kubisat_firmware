#include "unity.h"
#include "BH1750_WRAPPER.h"
#include "BME280_WRAPPER.h"

void setUp(void) {
    // Initialize hardware/mocks before each test
}

void tearDown(void) {
    // Clean up after each test
}

void test_bh1750_init(void) {
    BH1750Wrapper sensor;
    TEST_ASSERT_TRUE(sensor.init());
    TEST_ASSERT_TRUE(sensor.isInitialized());
}

void test_bme280_temperature_range(void) {
    BME280Wrapper sensor(i2c0);
    TEST_ASSERT_TRUE(sensor.init());
    float temp = sensor.readData(DataType::TEMPERATURE);
    TEST_ASSERT_FLOAT_WITHIN(100.0f, -40.0f, temp); // Valid temp range
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_bh1750_init);
    RUN_TEST(test_bme280_temperature_range);
    return UNITY_END();
}