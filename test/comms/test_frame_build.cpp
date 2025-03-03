// test_frame_build.cpp
#include "test_frame_common.h"

void test_frame_build_val() {
    Frame frame = frame_build(OperationType::VAL, 1, 2, "test_value", ValueUnit::VOLT);
    
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::VAL, frame.operationType);
    TEST_ASSERT_EQUAL(1, frame.group);
    TEST_ASSERT_EQUAL(2, frame.command);
    TEST_ASSERT_EQUAL_STRING("test_value", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("V", frame.unit.c_str());
}

void test_frame_build_err() {
    Frame frame = frame_build(OperationType::ERR, 1, 2, "error_message");
    
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::ERR, frame.operationType);
    TEST_ASSERT_EQUAL_STRING("error_message", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("", frame.unit.c_str());
}

void test_frame_build_get() {
    Frame frame = frame_build(OperationType::GET, 3, 4, "");
    
    TEST_ASSERT_EQUAL(0, frame.direction);  // Ground to satellite
    TEST_ASSERT_EQUAL(OperationType::GET, frame.operationType);
    TEST_ASSERT_EQUAL(3, frame.group);
    TEST_ASSERT_EQUAL(4, frame.command);
    TEST_ASSERT_EQUAL_STRING("", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("", frame.unit.c_str());
}

void test_frame_build_set() {
    Frame frame = frame_build(OperationType::SET, 5, 6, "set_value", ValueUnit::SECOND);
    
    TEST_ASSERT_EQUAL(0, frame.direction);  // Ground to satellite
    TEST_ASSERT_EQUAL(OperationType::SET, frame.operationType);
    TEST_ASSERT_EQUAL(5, frame.group);
    TEST_ASSERT_EQUAL(6, frame.command);
    TEST_ASSERT_EQUAL_STRING("set_value", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("s", frame.unit.c_str());  // Assuming SECOND is converted to "s"
}

void test_frame_build_res() {
    Frame frame = frame_build(OperationType::RES, 7, 8, "result", ValueUnit::BOOL);
    
    TEST_ASSERT_EQUAL(1, frame.direction);  // Satellite to ground
    TEST_ASSERT_EQUAL(OperationType::RES, frame.operationType);
    TEST_ASSERT_EQUAL(7, frame.group);
    TEST_ASSERT_EQUAL(8, frame.command);
    TEST_ASSERT_EQUAL_STRING("result", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("bool", frame.unit.c_str());  // Assuming BOOL is converted to "bool"
}

void test_frame_build_seq() {
    Frame frame = frame_build(OperationType::SEQ, 9, 10, "sequence_data", ValueUnit::TEXT);
    
    TEST_ASSERT_EQUAL(1, frame.direction);  // Satellite to ground
    TEST_ASSERT_EQUAL(OperationType::SEQ, frame.operationType);
    TEST_ASSERT_EQUAL(9, frame.group);
    TEST_ASSERT_EQUAL(10, frame.command);
    TEST_ASSERT_EQUAL_STRING("sequence_data", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("text", frame.unit.c_str());  // Assuming TEXT is converted to "text"
}