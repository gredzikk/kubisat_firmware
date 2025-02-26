// test/comms/test_error_codes.cpp
#include "unity.h"
#include "protocol.h"

void setUp(void) {}
void tearDown(void) {}

void test_error_code_conversion(void) {
    // Test error code to string conversion
    TEST_ASSERT_EQUAL_STRING("PARAM_UNNECESSARY", error_code_to_string(ErrorCode::PARAM_UNNECESSARY).c_str());
    TEST_ASSERT_EQUAL_STRING("PARAM_REQUIRED", error_code_to_string(ErrorCode::PARAM_REQUIRED).c_str());
    TEST_ASSERT_EQUAL_STRING("PARAM_INVALID", error_code_to_string(ErrorCode::PARAM_INVALID).c_str());
    TEST_ASSERT_EQUAL_STRING("INVALID_OPERATION", error_code_to_string(ErrorCode::INVALID_OPERATION).c_str());
    TEST_ASSERT_EQUAL_STRING("NOT_ALLOWED", error_code_to_string(ErrorCode::NOT_ALLOWED).c_str());
    TEST_ASSERT_EQUAL_STRING("INVALID_FORMAT", error_code_to_string(ErrorCode::INVALID_FORMAT).c_str());
    TEST_ASSERT_EQUAL_STRING("INVALID_VALUE", error_code_to_string(ErrorCode::INVALID_VALUE).c_str());
    TEST_ASSERT_EQUAL_STRING("UNKNOWN_ERROR", error_code_to_string(ErrorCode::UNKNOWN_ERROR).c_str());
}