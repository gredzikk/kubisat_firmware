// test_frame_build.cpp
#include "test_frame_common.h"

void test_frame_build_success() {
    Frame frame = frame_build(OperationType::VAL, 1, 2, "test_value", ValueUnit::VOLT);
    
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::ANS, frame.operationType);
    TEST_ASSERT_EQUAL(1, frame.group);
    TEST_ASSERT_EQUAL(2, frame.command);
    TEST_ASSERT_EQUAL_STRING("test_value", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("V", frame.unit.c_str());
}

void test_frame_build_error() {
    Frame frame = frame_build(OperationType::ERR, 1, 2, "error_message");
    
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::ERR, frame.operationType);
    TEST_ASSERT_EQUAL_STRING("error_message", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("", frame.unit.c_str());
}

void test_frame_build_info() {
    Frame frame = frame_build(OperationType::RES, 1, 2, "info_message");
    
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::INF, frame.operationType);
    TEST_ASSERT_EQUAL_STRING("info_message", frame.value.c_str());
}