// test/comms/test_command_handlers.cpp
#include "unity.h"
#include "protocol.h"
#include "communication.h"
#include "commands.h"

static bool uart_send_called = false;
static bool lora_send_called = false;
static Frame last_frame_sent;

void send_frame_uart(const Frame& frame) {
    uart_send_called = true;
    last_frame_sent = frame;
}

void send_frame_lora(const Frame& frame) {
    lora_send_called = true;
    last_frame_sent = frame;
}

void setUp(void) {
    uart_send_called = false;
    lora_send_called = false;
}

void tearDown(void) {
}

void test_command_handler_get_operation(void) {
    std::vector<Frame> response = handle_get_build_version("", OperationType::GET);
    
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(1, response[0].command);
    TEST_ASSERT_EQUAL(BUILD_NUMBER, std::stoi(response[0].value));
}

void test_command_handler_set_operation(void) {
    VerbosityLevel old_level = get_verbosity_level();
    std::vector<Frame> response = handle_verbosity("2", OperationType::SET);
    
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(8, response[0].command);
    TEST_ASSERT_EQUAL_STRING("LEVEL SET", response[0].value.c_str());
    
    TEST_ASSERT_EQUAL(VerbosityLevel::WARNING, get_verbosity_level());
    
    set_verbosity_level(old_level);
}

void test_command_handler_invalid_operation(void) {
    std::vector<Frame> response = handle_get_build_version("", OperationType::SET);
    
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::ERR, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(1, response[0].command);
}