// test_frame_converters.cpp
#include "test_frame_common.h"

void test_operation_type_conversion() {
    OperationType type = OperationType::GET;
    std::string str = operation_type_to_string(type);
    OperationType converted = string_to_operation_type(str);
    
    TEST_ASSERT_EQUAL(type, converted);
    TEST_ASSERT_EQUAL_STRING("GET", str.c_str());
}

void test_value_unit_type_conversion() {
    ValueUnit unit = ValueUnit::VOLT;
    std::string str = value_unit_type_to_string(unit);
    
    TEST_ASSERT_EQUAL_STRING("V", str.c_str());
}

void test_exception_type_conversion() {
    ExceptionType type = ExceptionType::INVALID_PARAM;
    std::string str = exception_type_to_string(type);
    
    TEST_ASSERT_EQUAL_STRING("INVALID PARAM", str.c_str());
}

void test_hex_string_conversion() {
    std::string hex = "0A0B0C";
    std::vector<uint8_t> bytes = hex_string_to_bytes(hex);
    
    TEST_ASSERT_EQUAL(3, bytes.size());
    TEST_ASSERT_EQUAL(0x0A, bytes[0]);
    TEST_ASSERT_EQUAL(0x0B, bytes[1]);
    TEST_ASSERT_EQUAL(0x0C, bytes[2]);
}