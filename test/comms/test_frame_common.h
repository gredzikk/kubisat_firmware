// test_frame_common.h
#ifndef TEST_FRAME_COMMON_H
#define TEST_FRAME_COMMON_H

#include "unity.h"
#include "protocol.h"
#include "communication.h"

// Helper function to create a test frame
Frame create_test_frame() {
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.direction = 1;
    frame.operationType = OperationType::GET;
    frame.group = 1;
    frame.command = 2;
    frame.value = "test_value";
    frame.unit = "V";
    frame.footer = FRAME_END;
    return frame;
}

#endif