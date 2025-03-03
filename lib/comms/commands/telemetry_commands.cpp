#include "commands.h"
#include "communication.h"
#include "telemetry_manager.h"

#define TELEMETRY_GROUP 8
#define GET_LAST_TELEMETRY_RECORD_COMMAND 2
#define GET_LAST_SENSOR_RECORD_COMMAND 3 

extern mutex_t telemetry_mutex;
extern size_t telemetry_buffer_count;
extern size_t telemetry_buffer_write_index;
extern const int TELEMETRY_BUFFER_SIZE; 

/**
 * @defgroup TelemetryBufferCommands Telemetry Buffer Commands
 * @brief Commands for interacting with the telemetry buffer.
 * @{
 */

/**
 * @brief Handles the get last record command.
 *
 * This function reads the last record from the telemetry buffer, base64 encodes it,
 * and sends the encoded data as a response.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A vector of Frames indicating the result of the operation.
 *         - Success: Frame with base64 encoded telemetry data.
 *         - Error: Frame with error message (e.g., "No telemetry data available").
 *
 * @note <b>KBST;0;GET;8;2;;TSBK</b>
 * @note This command retrieves the last telemetry record from the buffer and sends it base64 encoded.
 * @ingroup TelemetryBufferCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 8.2
 */
std::vector<Frame> handle_get_last_telemetry_record(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, TELEMETRY_GROUP, GET_LAST_TELEMETRY_RECORD_COMMAND, error_msg));
        return frames;
    }

    std::string csv_data = get_last_telemetry_record_csv();

    if (csv_data.empty()) {
        error_msg = "NO_DATA";
        frames.push_back(frame_build(OperationType::ERR, TELEMETRY_GROUP, GET_LAST_SENSOR_RECORD_COMMAND, error_msg));
        return frames;
    }
    // Create and send the frame with the base64 encoded data
    frames.push_back(frame_build(OperationType::VAL, TELEMETRY_GROUP, GET_LAST_TELEMETRY_RECORD_COMMAND, csv_data));

    return frames;
}


/**
 * @brief Handles the get last sensor record command.
 *
 * This function retrieves the last sensor record from the telemetry manager,
 * and sends the data as a response.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A vector of Frames indicating the result of the operation.
 */
std::vector<Frame> handle_get_last_sensor_record(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, TELEMETRY_GROUP, GET_LAST_SENSOR_RECORD_COMMAND, error_msg));
        return frames;
    }

    std::string csv_data = get_last_sensor_record_csv();

    if (csv_data.empty()) {
        error_msg = "NO_DATA";
        frames.push_back(frame_build(OperationType::ERR, TELEMETRY_GROUP, GET_LAST_SENSOR_RECORD_COMMAND, error_msg));
        return frames;
    }

    // Create and send the frame with the sensor data
    frames.push_back(frame_build(OperationType::VAL, TELEMETRY_GROUP, GET_LAST_SENSOR_RECORD_COMMAND, csv_data));

    return frames;
}
/** @} */ // TelemetryBufferCommands