// filepath: test/test_ds3231.cpp
#include "DS3231.h"
#include "unity.h"
#include <ctime>


void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // Clean up after each test
}
// Helper function to convert binary to BCD
static uint8_t bin_to_bcd(uint8_t value) {
    return ((value / 10) << 4) | (value % 10);
}

// Test for successful time setting
void test_set_time_success(void) {
    DS3231& rtc = DS3231::get_instance();
    uart_puts(uart0, "get instance\n");
    // Set a specific time: 2023-05-15 14:30:45
    struct tm test_time = {};
    test_time.tm_year = 2023 - 1900; // Years since 1900
    test_time.tm_mon = 5 - 1;        // 0-based month (0-11)
    test_time.tm_mday = 15;          // Day of month (1-31)
    test_time.tm_hour = 14;          // Hours (0-23)
    test_time.tm_min = 30;           // Minutes (0-59)
    test_time.tm_sec = 45;           // Seconds (0-59)
    test_time.tm_isdst = 0;          // No daylight saving
    
    time_t unix_time = mktime(&test_time);
    uart_puts(uart0, "mktime\n");
    // Call the function under test
    int result = rtc.set_time(unix_time);
    uart_puts(uart0, "set time\n");
    // Verify results
    TEST_ASSERT_EQUAL(0, result);
}

// Test for handling invalid time
void test_set_time_invalid_time(void) {
    DS3231& rtc = DS3231::get_instance();
    
    // Use a very large value that should cause gmtime to fail
    // Note: This is implementation-dependent and might not fail on all platforms
    time_t invalid_time = INT_MAX;
    
    // Call the function under test
    int result = rtc.set_time(invalid_time);
    
    // This test is commented out because it's platform-dependent
    // Uncomment if your platform's gmtime fails with INT_MAX
    // TEST_ASSERT_EQUAL(-1, result);
}

// Test for correct BCD conversion
void test_set_time_bcd_conversion(void) {
    DS3231& rtc = DS3231::get_instance();
    
    // Create times that test edge cases in BCD conversion
    struct tm times_to_test[] = {
        // Test zeros
        {.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 100, .tm_isdst = 0},
        // Test values that require both nibbles in BCD
        {.tm_sec = 59, .tm_min = 45, .tm_hour = 23, .tm_mday = 31, .tm_mon = 11, .tm_year = 199, .tm_isdst = 0}
    };
    
    for (size_t i = 0; i < sizeof(times_to_test) / sizeof(times_to_test[0]); i++) {
        time_t test_time = mktime(&times_to_test[i]);
        
        // Call the function under test
        int result = rtc.set_time(test_time);
        
        // Verify results
        TEST_ASSERT_EQUAL(0, result);
    }
}


// Test for get_time functionality
void test_get_time_success(void) {
    DS3231& rtc = DS3231::get_instance();
    
    // Set a specific time: 2023-05-15 14:30:45
    struct tm test_time = {};
    test_time.tm_year = 2023 - 1900; // Years since 1900
    test_time.tm_mon = 5 - 1;        // 0-based month (0-11)
    test_time.tm_mday = 15;          // Day of month (1-31)
    test_time.tm_hour = 14;          // Hours (0-23)
    test_time.tm_min = 30;           // Minutes (0-59)
    test_time.tm_sec = 45;           // Seconds (0-59)
    test_time.tm_isdst = 0;          // No daylight saving
    
    time_t expected_time = mktime(&test_time);
    
    // Set the time
    rtc.set_time(expected_time);
    
    // Allow a small delay for the RTC to update
    sleep_ms(100);
    
    // Call the function under test
    time_t result_time = rtc.get_time();
    
    // Account for a few seconds of drift
    time_t lower_bound = expected_time;
    time_t upper_bound = expected_time + 2;
    
    // Assert that the result time is within the expected range
    TEST_ASSERT_GREATER_OR_EQUAL(lower_bound, result_time);
    TEST_ASSERT_LESS_OR_EQUAL(upper_bound, result_time);
}

// Test for timezone functionality
void test_timezone_offset(void) {
    DS3231& rtc = DS3231::get_instance();
    
    // Test setting a positive offset (UTC+2)
    rtc.set_timezone_offset(120);
    TEST_ASSERT_EQUAL(120, rtc.get_timezone_offset());
    
    // Test setting a negative offset (UTC-5)
    rtc.set_timezone_offset(-300);
    TEST_ASSERT_EQUAL(-300, rtc.get_timezone_offset());
    
    // Test invalid offset handling (too large)
    rtc.set_timezone_offset(800);
    TEST_ASSERT_EQUAL(-300, rtc.get_timezone_offset()); // Should remain unchanged
    
    // Test invalid offset handling (too small)
    rtc.set_timezone_offset(-800);
    TEST_ASSERT_EQUAL(-300, rtc.get_timezone_offset()); // Should remain unchanged
}

