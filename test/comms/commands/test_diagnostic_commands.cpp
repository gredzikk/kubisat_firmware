#include "unity.h"
#include "commands.h"
#include "protocol.h"
#include "build_number.h"

void test_handle_get_commands_list(void) {
    std::vector<Frame> response = handle_get_commands_list("", OperationType::GET);
    
    TEST_ASSERT_TRUE(response.size() > 0);
    TEST_ASSERT_EQUAL(OperationType::SEQ, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(0, response[0].command);
}

void test_handle_get_build_version(void) {
    std::vector<Frame> response = handle_get_build_version("", OperationType::GET);
    
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(1, response[0].command);
    TEST_ASSERT_EQUAL(BUILD_NUMBER, std::stoi(response[0].value));
}

void test_handle_verbosity(void) {
    // Test SET operation
    std::vector<Frame> response = handle_verbosity("2", OperationType::SET);
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(8, response[0].command);
    TEST_ASSERT_EQUAL_STRING("LEVEL SET", response[0].value.c_str());
    
    // Test GET operation
    response = handle_verbosity("", OperationType::GET);
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
}

void test_handle_enter_bootloader_mode(void) {
    std::vector<Frame> response = handle_enter_bootloader_mode("", OperationType::SET);
    
    TEST_ASSERT_EQUAL(1, response.size());
    TEST_ASSERT_EQUAL(OperationType::VAL, response[0].operationType);
    TEST_ASSERT_EQUAL(1, response[0].group);
    TEST_ASSERT_EQUAL(9, response[0].command);
}