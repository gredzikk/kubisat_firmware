#include "communication.h"
#include "ISensor.h"
#include <vector>
#include <string>
#include <sstream>
#include "commands.h"

#define SENSOR_GROUP 4
#define SENSOR_READ 0
#define SENSOR_CONFIGURE 1

/**
 * @defgroup SensorCommands Sensor Commands
 * @brief Commands for reading and configuring sensors
 * @{
 */

/**
 * @brief Handler for reading sensor data
 * @param param String in format "sensor_type[-data_type]" where:
 *        - sensor_type: "light", "environment", "magnetometer", "imu"
 *        - data_type (optional): specific data type for the sensor
 *          - For light: "light_level"
 *          - For environment: "temperature", "pressure", "humidity"
 *          - For magnetometer: "mag_field_x", "mag_field_y", "mag_field_z"
 *          - For IMU: "gyro_x", "gyro_y", "gyro_z", "accel_x", "accel_y", "accel_z"
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Sensor data value(s)
 *         - Error: Error message
 * @note <b>KBST;0;GET;3;0;light-light_level;TSBK</b>
 * @note This command is used to read data from sensors
 * @ingroup SensorCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 3.0
 */
std::vector<Frame> handle_get_sensor_data(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_REQUIRED);
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_READ, error_msg));
        return frames;
    }

    if (operationType != OperationType::GET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_READ, error_msg));
        return frames;
    }

    // Parse sensor type and data type from param
    std::string sensor_type_str;
    std::string data_type_str;
    
    size_t dash_pos = param.find('-');
    if (dash_pos != std::string::npos) {
        sensor_type_str = param.substr(0, dash_pos);
        data_type_str = param.substr(dash_pos + 1);
    } else {
        sensor_type_str = param;
    }

    // Convert sensor_type_str to SensorType
    SensorType sensor_type;
    if (sensor_type_str == "light") {
        sensor_type = SensorType::LIGHT;
    } else if (sensor_type_str == "environment") {
        sensor_type = SensorType::ENVIRONMENT;
    } else if (sensor_type_str == "magnetometer") {
        sensor_type = SensorType::MAGNETOMETER;
    } else if (sensor_type_str == "imu") {
        sensor_type = SensorType::IMU;
    } else {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID) + ": Invalid sensor type";
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_READ, error_msg));
        return frames;
    }

    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    
    // If data type is specified, read that specific data
    if (!data_type_str.empty()) {
        SensorDataTypeIdentifier data_type;
        
        // Map string to SensorDataTypeIdentifier
        if (data_type_str == "light_level") {
            data_type = SensorDataTypeIdentifier::LIGHT_LEVEL;
        } else if (data_type_str == "temperature") {
            data_type = SensorDataTypeIdentifier::TEMPERATURE;
        } else if (data_type_str == "pressure") {
            data_type = SensorDataTypeIdentifier::PRESSURE;
        } else if (data_type_str == "humidity") {
            data_type = SensorDataTypeIdentifier::HUMIDITY;
        } else if (data_type_str == "mag_field_x") {
            data_type = SensorDataTypeIdentifier::MAG_FIELD_X;
        } else if (data_type_str == "mag_field_y") {
            data_type = SensorDataTypeIdentifier::MAG_FIELD_Y;
        } else if (data_type_str == "mag_field_z") {
            data_type = SensorDataTypeIdentifier::MAG_FIELD_Z;
        } else if (data_type_str == "gyro_x") {
            data_type = SensorDataTypeIdentifier::GYRO_X;
        } else if (data_type_str == "gyro_y") {
            data_type = SensorDataTypeIdentifier::GYRO_Y;
        } else if (data_type_str == "gyro_z") {
            data_type = SensorDataTypeIdentifier::GYRO_Z;
        } else if (data_type_str == "accel_x") {
            data_type = SensorDataTypeIdentifier::ACCEL_X;
        } else if (data_type_str == "accel_y") {
            data_type = SensorDataTypeIdentifier::ACCEL_Y;
        } else if (data_type_str == "accel_z") {
            data_type = SensorDataTypeIdentifier::ACCEL_Z;
        } else {
            error_msg = error_code_to_string(ErrorCode::PARAM_INVALID) + ": Invalid data type";
            frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_READ, error_msg));
            return frames;
        }
        
        float value = sensor_wrapper.sensor_read_data(sensor_type, data_type);
        std::stringstream ss;
        ss << value;
        frames.push_back(frame_build(OperationType::VAL, SENSOR_GROUP, SENSOR_READ, ss.str()));
    } 
    // If only sensor type is specified, read all relevant data for that sensor type
    else {
        std::vector<SensorDataTypeIdentifier> data_types;
        
        switch (sensor_type) {
            case SensorType::LIGHT:
                data_types = {SensorDataTypeIdentifier::LIGHT_LEVEL};
                break;
            case SensorType::ENVIRONMENT:
                data_types = {
                    SensorDataTypeIdentifier::TEMPERATURE,
                    SensorDataTypeIdentifier::PRESSURE,
                    SensorDataTypeIdentifier::HUMIDITY
                };
                break;
            case SensorType::MAGNETOMETER:
                data_types = {
                    SensorDataTypeIdentifier::MAG_FIELD_X,
                    SensorDataTypeIdentifier::MAG_FIELD_Y,
                    SensorDataTypeIdentifier::MAG_FIELD_Z
                };
                break;
            case SensorType::IMU:
                data_types = {
                    SensorDataTypeIdentifier::GYRO_X,
                    SensorDataTypeIdentifier::GYRO_Y,
                    SensorDataTypeIdentifier::GYRO_Z,
                    SensorDataTypeIdentifier::ACCEL_X,
                    SensorDataTypeIdentifier::ACCEL_Y,
                    SensorDataTypeIdentifier::ACCEL_Z
                };
                break;
        }
        
        std::stringstream combined_values;
        std::vector<std::string> data_type_names;
        std::vector<float> values;
        
        // Get names for the data types and store the values
        for (SensorDataTypeIdentifier data_type : data_types) {
            switch (data_type) {
                case SensorDataTypeIdentifier::LIGHT_LEVEL:
                    data_type_names.push_back("light_level");
                    break;
                case SensorDataTypeIdentifier::TEMPERATURE:
                    data_type_names.push_back("temperature");
                    break;
                case SensorDataTypeIdentifier::PRESSURE:
                    data_type_names.push_back("pressure");
                    break;
                case SensorDataTypeIdentifier::HUMIDITY:
                    data_type_names.push_back("humidity");
                    break;
                case SensorDataTypeIdentifier::MAG_FIELD_X:
                    data_type_names.push_back("mag_field_x");
                    break;
                case SensorDataTypeIdentifier::MAG_FIELD_Y:
                    data_type_names.push_back("mag_field_y");
                    break;
                case SensorDataTypeIdentifier::MAG_FIELD_Z:
                    data_type_names.push_back("mag_field_z");
                    break;
                case SensorDataTypeIdentifier::GYRO_X:
                    data_type_names.push_back("gyro_x");
                    break;
                case SensorDataTypeIdentifier::GYRO_Y:
                    data_type_names.push_back("gyro_y");
                    break;
                case SensorDataTypeIdentifier::GYRO_Z:
                    data_type_names.push_back("gyro_z");
                    break;
                case SensorDataTypeIdentifier::ACCEL_X:
                    data_type_names.push_back("accel_x");
                    break;
                case SensorDataTypeIdentifier::ACCEL_Y:
                    data_type_names.push_back("accel_y");
                    break;
                case SensorDataTypeIdentifier::ACCEL_Z:
                    data_type_names.push_back("accel_z");
                    break;
            }
            
            float value = sensor_wrapper.sensor_read_data(sensor_type, data_type);
            values.push_back(value);
        }
        
        // Format output as key-value pairs
        for (size_t i = 0; i < data_type_names.size(); i++) {
            if (i > 0) combined_values << "|";
            combined_values << data_type_names[i] << ":" << values[i];
        }
        
        frames.push_back(frame_build(OperationType::VAL, SENSOR_GROUP, SENSOR_READ, combined_values.str()));
    }
    
    return frames;
}

/**
 * @brief Handler for configuring sensors
 * @param param String in format "sensor_type;key1:value1|key2:value2|..."
 *        - sensor_type: "light", "environment", "magnetometer", "imu"
 *        - key-value pairs for configuration parameters
 * @param operationType SET
 * @return Vector of Frames containing:
 *         - Success: Success message
 *         - Error: Error message
 * @note <b>KBST;0;SET;3;1;light;measurement_mode:continuously_high_resolution;TSBK</b>
 * @note This command is used to configure sensors
 * @ingroup SensorCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 3.1
 */
std::vector<Frame> handle_sensor_config(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (param.empty()) {
        error_msg = error_code_to_string(ErrorCode::PARAM_REQUIRED);
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_CONFIGURE, error_msg));
        return frames;
    }

    if (operationType != OperationType::SET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_CONFIGURE, error_msg));
        return frames;
    }

    // Parse sensor type and configuration from param
    size_t semicolon_pos = param.find(';');
    if (semicolon_pos == std::string::npos) {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID) + ": Format should be sensor_type;config_params";
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_CONFIGURE, error_msg));
        return frames;
    }

    std::string sensor_type_str = param.substr(0, semicolon_pos);
    std::string config_str = param.substr(semicolon_pos + 1);

    // Convert sensor_type_str to SensorType
    SensorType sensor_type;
    if (sensor_type_str == "light") {
        sensor_type = SensorType::LIGHT;
    } else if (sensor_type_str == "environment") {
        sensor_type = SensorType::ENVIRONMENT;
    } else if (sensor_type_str == "magnetometer") {
        sensor_type = SensorType::MAGNETOMETER;
    } else if (sensor_type_str == "imu") {
        sensor_type = SensorType::IMU;
    } else {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID) + ": Invalid sensor type";
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_CONFIGURE, error_msg));
        return frames;
    }

    // Parse configuration parameters
    std::map<std::string, std::string> config_map;
    std::stringstream ss(config_str);
    std::string config_pair;
    
    while (std::getline(ss, config_pair, '|')) {
        size_t colon_pos = config_pair.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = config_pair.substr(0, colon_pos);
            std::string value = config_pair.substr(colon_pos + 1);
            config_map[key] = value;
        }
    }

    // Apply configuration
    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    bool success = sensor_wrapper.sensor_configure(sensor_type, config_map);

    if (success) {
        frames.push_back(frame_build(OperationType::RES, SENSOR_GROUP, SENSOR_CONFIGURE, "Configuration successful"));
    } else {
        error_msg = error_code_to_string(ErrorCode::FAIL_TO_SET) + ": Failed to configure sensor";
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, SENSOR_CONFIGURE, error_msg));
    }
    
    return frames;
}


/**
 * @brief Handler for listing available sensors
 * @param param Empty string or optional filter criteria
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: List of available sensors
 *         - Error: Error message
 * @note <b>KBST;0;GET;4;2;;TSBK</b> (lists all sensors)
 * @note This command is used to get a list of available sensors
 * @ingroup SensorCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 4.2
 */
std::vector<Frame> handle_get_sensor_list(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, SENSOR_GROUP, 2, error_msg));
        return frames;
    }

    // Get the singleton instance
    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    
    // Get list of available sensor types
    std::vector<std::pair<SensorType, uint8_t>> available_sensors = sensor_wrapper.get_available_sensors();
    
    if (available_sensors.empty()) {
        frames.push_back(frame_build(OperationType::VAL, SENSOR_GROUP, 2, "No sensors available"));
        return frames;
    }
    
    std::stringstream sensor_list;
    bool first = true;
    
    for (const auto& sensor_info : available_sensors) {
        if (!first) {
            sensor_list << "|";
        }
        
        // Format: sensor_type:address (in hex)
        std::stringstream addr_hex;
        addr_hex << std::hex << static_cast<int>(sensor_info.second);
        
        switch (sensor_info.first) {
            case SensorType::LIGHT:
                sensor_list << "light:0x" << addr_hex.str();
                break;
            case SensorType::ENVIRONMENT:
                sensor_list << "environment:0x" << addr_hex.str();
                break;
            case SensorType::MAGNETOMETER:
                sensor_list << "magnetometer:0x" << addr_hex.str();
                break;
            case SensorType::IMU:
                sensor_list << "imu:0x" << addr_hex.str();
                break;
            default:
                sensor_list << "unknown:0x" << addr_hex.str();
                break;
        }
        
        first = false;
    }
    
    frames.push_back(frame_build(OperationType::VAL, SENSOR_GROUP, 2, sensor_list.str()));
    return frames;
}