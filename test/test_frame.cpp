#include "unity.h"
#include "communication.h"
#include <string>

void test_frame_encode_decode() {
    Frame original_frame;
    original_frame.header = FRAME_BEGIN;
    original_frame.direction = 0;
    original_frame.operationType = OperationType::GET;
    original_frame.group = 1;
    original_frame.command = 1;
    original_frame.value = "test_value";
    original_frame.unit = "V";
    original_frame.footer = FRAME_END;

    std::string encoded_frame = frame_encode(original_frame);
    Frame decoded_frame = frame_decode(encoded_frame);

    TEST_ASSERT_EQUAL_STRING(original_frame.header.c_str(), decoded_frame.header.c_str());
    TEST_ASSERT_NOT_EQUAL(original_frame.direction, decoded_frame.direction);
    TEST_ASSERT_EQUAL(original_frame.operationType, decoded_frame.operationType);
    TEST_ASSERT_EQUAL(original_frame.group, decoded_frame.group);
    TEST_ASSERT_EQUAL(original_frame.command, decoded_frame.command);
    TEST_ASSERT_EQUAL_STRING(original_frame.value.c_str(), decoded_frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING(original_frame.unit.c_str(), decoded_frame.unit.c_str());
    TEST_ASSERT_EQUAL_STRING(original_frame.footer.c_str(), decoded_frame.footer.c_str());
}

void test_frame_encode_decode_no_unit() {
    Frame original_frame;
    original_frame.header = FRAME_BEGIN;
    original_frame.direction = 1;
    original_frame.operationType = OperationType::RES;
    original_frame.group = 2;
    original_frame.command = 5;
    original_frame.value = "12345";
    original_frame.unit = "";
    original_frame.footer = FRAME_END;

    std::string encoded_frame = frame_encode(original_frame);
    Frame decoded_frame = frame_decode(encoded_frame);

    TEST_ASSERT_EQUAL_STRING(original_frame.header.c_str(), decoded_frame.header.c_str());
    TEST_ASSERT_EQUAL(original_frame.direction, decoded_frame.direction);
    TEST_ASSERT_EQUAL(original_frame.operationType, decoded_frame.operationType);
    TEST_ASSERT_EQUAL(original_frame.group, decoded_frame.group);
    TEST_ASSERT_EQUAL(original_frame.command, decoded_frame.command);
    TEST_ASSERT_EQUAL_STRING(original_frame.value.c_str(), decoded_frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING(original_frame.unit.c_str(), decoded_frame.unit.c_str());
    TEST_ASSERT_EQUAL_STRING(original_frame.footer.c_str(), decoded_frame.footer.c_str());
}

void test_frame_decode_invalid_header() {
    std::string invalid_frame_data = "INVALID;0;GET;1;1;test_value;mV;TSBK";
    Frame decoded_frame = frame_decode(invalid_frame_data);
    TEST_ASSERT_EQUAL(OperationType::ERR, decoded_frame.operationType);
}

void test_frame_decode_missing_footer() {
    std::string invalid_frame_data = "KBST;0;GET;1;1;test_value;mV;";
    Frame decoded_frame = frame_decode(invalid_frame_data);
    TEST_ASSERT_EQUAL(OperationType::ERR, decoded_frame.operationType);
}

void test_frame_build_val() {
    Frame frame = frame_build(OperationType::VAL, 1, 2, "42", ValueUnit::CELSIUS);
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::VAL, frame.operationType);
    TEST_ASSERT_EQUAL(1, frame.group);
    TEST_ASSERT_EQUAL(2, frame.command);
    TEST_ASSERT_EQUAL_STRING("42", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("C", frame.unit.c_str());
}

void test_frame_build_err() {
    Frame frame = frame_build(OperationType::ERR, 0, 0, "Generic error");
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::ERR, frame.operationType);
    TEST_ASSERT_EQUAL(0, frame.group);
    TEST_ASSERT_EQUAL(0, frame.command);
    TEST_ASSERT_EQUAL_STRING("Generic error", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("", frame.unit.c_str());
}

void test_frame_build_res() {
    Frame frame = frame_build(OperationType::RES, 3, 4, "25.5", ValueUnit::VOLT);
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::RES, frame.operationType);
    TEST_ASSERT_EQUAL(3, frame.group);
    TEST_ASSERT_EQUAL(4, frame.command);
    TEST_ASSERT_EQUAL_STRING("25.5", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("V", frame.unit.c_str());
}

void test_frame_build_seq() {
    Frame frame = frame_build(OperationType::SEQ, 5, 6, "next_sequence");
    TEST_ASSERT_EQUAL(1, frame.direction);
    TEST_ASSERT_EQUAL(OperationType::SEQ, frame.operationType);
    TEST_ASSERT_EQUAL(5, frame.group);
    TEST_ASSERT_EQUAL(6, frame.command);
    TEST_ASSERT_EQUAL_STRING("next_sequence", frame.value.c_str());
    TEST_ASSERT_EQUAL_STRING("", frame.unit.c_str());
}

void test_frame_decode_invalid_operation_type() {
    std::string invalid_frame_data = "KBST;0;BOGUS;1;1;test_value;mV;TSBK";
    Frame decoded_frame = frame_decode(invalid_frame_data);
    TEST_ASSERT_EQUAL(OperationType::ERR, decoded_frame.operationType);
}

void test_frame_decode_empty_data() {
    std::string empty_frame_data = "";
    Frame decoded_frame = frame_decode(empty_frame_data);
    TEST_ASSERT_EQUAL(OperationType::ERR, decoded_frame.operationType);
}