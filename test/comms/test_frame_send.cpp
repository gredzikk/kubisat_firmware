// test/comms/test_frame_send.cpp
#include "unity.h"
#include "communication.h"
#include "../mocks/hardware_mocks.h"

void setUp(void) {
    // Enable mocks before each test
    mock_uart_enabled = true;
    uart_output_buffer.clear();
}

void tearDown(void) {
    // Disable mocks after each test
    mock_uart_enabled = false;
}

void test_send_frame_uart(void) {
    // Create a test frame
    Frame test_frame = {
        .operationType = OperationType::VAL,
        .group = 1,
        .command = 2,
        .value = "TEST_VALUE"
    };
    
    // Call function under test
    send_frame_uart(test_frame);
    
    // Verify output using mocks
    TEST_ASSERT_EQUAL(1, uart_output_buffer.size());
    TEST_ASSERT_TRUE(uart_output_buffer[0].find("KBST;0;VAL;1;2;TEST_VALUE;") != std::string::npos);
}