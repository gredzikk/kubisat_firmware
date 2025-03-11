// test_frame_codec.cpp
#include "test_frame_common.h"

void test_frame_encode_basic() {
    uart_puts(uart0, "start frame_encode_basic");
    Frame frame = create_test_frame();
    std::string encoded = frame_encode(frame);
    
    TEST_ASSERT_NOT_EQUAL(0, encoded.length());
    TEST_ASSERT_TRUE(encoded.find(FRAME_BEGIN) != std::string::npos);
    TEST_ASSERT_TRUE(encoded.find(FRAME_END) != std::string::npos);
    TEST_ASSERT_TRUE(encoded.find("test_value") != std::string::npos);
    uart_puts(uart0, "stop frame_encode_basic");
}

void test_frame_decode_basic() {
    
    Frame original = create_test_frame();
    std::string encoded = frame_encode(original);
    Frame decoded = frame_decode(encoded);
    
    TEST_ASSERT_EQUAL(original.direction, decoded.direction);
    TEST_ASSERT_EQUAL(original.group, decoded.group);
    TEST_ASSERT_EQUAL(original.command, decoded.command);
    TEST_ASSERT_EQUAL_STRING(original.value.c_str(), decoded.value.c_str());
    TEST_ASSERT_EQUAL_STRING(original.unit.c_str(), decoded.unit.c_str());
}

void test_frame_decode_invalid_header() {
    std::string invalid_frame = "INVALID" + std::string(1, DELIMITER) + "rest_of_frame";
    bool exceptionThrown = false;

    try {
        Frame decoded = frame_decode(invalid_frame);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
    } catch (...) {
        // Catch any other exceptions to avoid crashing the test
    }

    TEST_ASSERT_TRUE(exceptionThrown);
}