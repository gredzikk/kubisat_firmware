#include "telemetry_manager.h"
#include "utils.h"
#include "storage.h"
#include "PowerManager.h"
#include "ISensor.h"
#include "DS3231.h"
#include <deque>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include "communication.h"
#include "system_state_manager.h"

/**
 * @file telemetry_manager.cpp
 * @brief Implementation of telemetry collection and storage functionality
 * @details Handles collecting, buffering, and persisting telemetry data from various 
 *          satellite subsystems including power, sensors, and GPS
 */


extern PowerManager powerManager;
extern DS3231 systemClock;


/**
 * @brief Path to the telemetry CSV file on storage media
 */
#define TELEMETRY_CSV_PATH "/telemetry.csv"

/**
 * @brief Path to the sensor data CSV file on storage media
 */
#define SENSOR_DATA_CSV_PATH "/sensors.csv"

/**
 * @brief Default interval between telemetry samples in milliseconds (2 seconds)
 */
#define DEFAULT_SAMPLE_INTERVAL_MS 1000

/**
 * @brief Default number of records to collect before flushing to storage
 */
#define DEFAULT_FLUSH_THRESHOLD 10

/**
 * @brief Current sampling interval in milliseconds
 */
static uint32_t sample_interval_ms = DEFAULT_SAMPLE_INTERVAL_MS;

/**
 * @brief Current flush threshold (number of records that triggers a flush)
 */
static uint32_t flush_threshold = DEFAULT_FLUSH_THRESHOLD;

/**
 * @brief Circular buffer for telemetry records
 */
TelemetryRecord telemetry_buffer[TELEMETRY_BUFFER_SIZE];
size_t telemetry_buffer_count = 0;
size_t telemetry_buffer_write_index = 0;

/**
 * @brief Circular buffer for sensor data records
 */
SensorDataRecord sensor_data_buffer[TELEMETRY_BUFFER_SIZE];
size_t sensor_data_buffer_count = 0;
size_t sensor_data_buffer_write_index = 0;

/**
 * @brief Mutex for thread-safe access to the telemetry buffer
 */
mutex_t telemetry_mutex;

bool telemetry_init() {
    mutex_init(&telemetry_mutex);
    
    if (!SystemStateManager::get_instance().is_sd_card_mounted()) {
        bool success = true;
        
        FILE* file = fopen(TELEMETRY_CSV_PATH, "w");
        if (!file) {
            file = fopen(TELEMETRY_CSV_PATH, "w");
            if (file) {
                fprintf(file, "timestamp,build,battery_v,system_v,usb_ma,solar_ma,discharge_ma,"
                              "gps_time,latitude,lat_dir,longitude,lon_dir,speed_knots,course_deg,date,"
                              "fix_quality,satellites,altitude_m\n");
                fclose(file);
                uart_print("Created new telemetry log", VerbosityLevel::INFO);
            } else {
                uart_print("Failed to create telemetry log", VerbosityLevel::ERROR);
                success = false;
            }
        } else {
            fclose(file);
        }
        
        file = fopen(SENSOR_DATA_CSV_PATH, "w");
        if (!file) {
            file = fopen(SENSOR_DATA_CSV_PATH, "w");
            if (file) {
                fprintf(file, "timestamp,temperature,pressure,humidity,light\n");
                fclose(file);
                uart_print("Created new sensor data log", VerbosityLevel::INFO);
            } else {
                uart_print("Failed to create sensor data log", VerbosityLevel::ERROR);
                success = false;
            }
        } else {
            fclose(file);
        }
        
        return success;
    }
    
    uart_print("Telemetry system initialized (storage not available)", VerbosityLevel::WARNING);
    return false;
}

bool collect_telemetry() {
    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    uint32_t timestamp = systemClock.get_unix_time();
    TelemetryRecord record;
    record.timestamp = timestamp;
    record.build_version = std::to_string(BUILD_NUMBER);
    
    // Power data
    record.battery_voltage = powerManager.get_voltage_battery();
    record.system_voltage = powerManager.get_voltage_5v();
    record.charge_current_usb = powerManager.get_current_charge_usb();
    record.charge_current_solar = powerManager.get_current_charge_solar();
    record.discharge_current = powerManager.get_current_draw();
    
    auto& nmea_data = NMEAData::get_instance();
    // Get GPS RMC data
    std::vector<std::string> rmc_tokens = nmea_data.get_rmc_tokens();
    if (rmc_tokens.size() >= 12) {  // RMC has at least 12 fields when complete
        record.time = rmc_tokens[1];
        record.latitude = rmc_tokens[3];
        record.lat_dir = rmc_tokens[4];
        record.longitude = rmc_tokens[5];
        record.lon_dir = rmc_tokens[6];
        record.speed = rmc_tokens[7];
        record.course = rmc_tokens[8];
        record.date = rmc_tokens[9];
    } else {
        // Fill with defaults if no GPS data
        record.time = "";
        record.latitude = "";
        record.lat_dir = "";
        record.longitude = "";
        record.lon_dir = "";
        record.speed = "";
        record.course = "";
        record.date = "";
    }
    
    // Get GPS GGA data
    std::vector<std::string> gga_tokens = nmea_data.get_gga_tokens();
    if (gga_tokens.size() >= 15) {  // GGA has 15 fields when complete
        record.fix_quality = gga_tokens[6];
        record.satellites = gga_tokens[7];
        record.altitude = gga_tokens[9];
    } else {
        // Fill with defaults if no GPS data
        record.fix_quality = "";
        record.satellites = "";
        record.altitude = "";
    }
    
    SensorDataRecord sensor_record;
    sensor_record.timestamp = timestamp;
    sensor_record.temperature = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::TEMPERATURE);
    sensor_record.pressure = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::PRESSURE);
    sensor_record.humidity = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::HUMIDITY);
    sensor_record.light = sensor_wrapper.sensor_read_data(SensorType::LIGHT, SensorDataTypeIdentifier::LIGHT_LEVEL);
    
    mutex_enter_blocking(&telemetry_mutex);
    
    telemetry_buffer[telemetry_buffer_write_index] = record;
    sensor_data_buffer[telemetry_buffer_write_index] = sensor_record; 
    telemetry_buffer_write_index = (telemetry_buffer_write_index + 1) % TELEMETRY_BUFFER_SIZE;
    if (telemetry_buffer_count < TELEMETRY_BUFFER_SIZE) {
        telemetry_buffer_count++;
    }
    
    mutex_exit(&telemetry_mutex);

    uart_print("Telemetry collected", VerbosityLevel::DEBUG);

    return true;
}

bool flush_telemetry() {
    if (!SystemStateManager::get_instance().is_sd_card_mounted()) {
        return false;
    }
    
    
    mutex_enter_blocking(&telemetry_mutex);
    
    if (telemetry_buffer_count == 0) {
        mutex_exit(&telemetry_mutex);
        return true; // Nothing to save
    }
    
    FILE* file = fopen(TELEMETRY_CSV_PATH, "a");
    if (!file) {
        uart_print("Failed to open telemetry log for writing", VerbosityLevel::ERROR);
        mutex_exit(&telemetry_mutex);
        return false;
    }
    
    // Calculate start index (for circular buffer)
    size_t read_index = 0;
    if (telemetry_buffer_count == TELEMETRY_BUFFER_SIZE) {
        // Buffer is full, start from oldest entry
        read_index = telemetry_buffer_write_index;
    }
    
    // Write all records to CSV
    for (size_t i = 0; i < telemetry_buffer_count; i++) {
        fprintf(file, "%s\n", telemetry_buffer[read_index].to_csv().c_str());
        read_index = (read_index + 1) % TELEMETRY_BUFFER_SIZE;
    }
    
    // Clear buffer after successful write
    telemetry_buffer_count = 0;
    telemetry_buffer_write_index = 0;
    
    fclose(file);
    
    mutex_exit(&telemetry_mutex);
    return true;
}

bool flush_sensor_data() {
    if (!SystemStateManager::get_instance().is_sd_card_mounted()) {
        return false;
    }
    
    mutex_enter_blocking(&telemetry_mutex);
    
    if (telemetry_buffer_count == 0) {
        mutex_exit(&telemetry_mutex);
        return true; // Nothing to save
    }
    
    FILE* file = fopen(SENSOR_DATA_CSV_PATH, "a");
    if (!file) {
        uart_print("Failed to open sensor data log for writing", VerbosityLevel::ERROR);
        mutex_exit(&telemetry_mutex);
        return false;
    }
    
    // Calculate start index (for circular buffer)
    size_t read_index = 0;
    if (telemetry_buffer_count == TELEMETRY_BUFFER_SIZE) {
        // Buffer is full, start from oldest entry
        read_index = telemetry_buffer_write_index;
    }
    
    // Write all records to CSV
    for (size_t i = 0; i < telemetry_buffer_count; i++) {
        fprintf(file, "%s\n", sensor_data_buffer[read_index].to_csv().c_str());
        read_index = (read_index + 1) % TELEMETRY_BUFFER_SIZE;
    }
    
    // We don't need to clear the buffer here since it's cleared in flush_telemetry()
    // and both buffers use the same indices
    
    fclose(file);
    
    mutex_exit(&telemetry_mutex);
    return true;
}

bool is_telemetry_collection_time(uint32_t current_time, uint32_t& last_collection_time) {
    if (current_time - last_collection_time >= sample_interval_ms) {
        last_collection_time = current_time;
        return true;
    }
    return false;
}

bool is_telemetry_flush_time(uint32_t& collection_counter) {
    if (collection_counter >= flush_threshold) {
        collection_counter = 0;
        return true;
    }
    return false;
}

uint32_t get_telemetry_sample_interval() {
    return sample_interval_ms;
}

void set_telemetry_sample_interval(uint32_t interval_ms) {
    if (interval_ms >= 100) { // Minimum 100ms
        sample_interval_ms = interval_ms;
    }
}

uint32_t get_telemetry_flush_threshold() {
    return flush_threshold;
}

void set_telemetry_flush_threshold(uint32_t records) {
    if (records >= 1 && records <= 100) { 
        flush_threshold = records;
    }
}

std::string get_last_telemetry_record_csv() {
    mutex_enter_blocking(&telemetry_mutex);
    
    if (telemetry_buffer_count == 0) {
        mutex_exit(&telemetry_mutex);
        return ""; 
    }
    
    size_t last_record_index = (telemetry_buffer_write_index + TELEMETRY_BUFFER_SIZE - 1) % TELEMETRY_BUFFER_SIZE;
    
    TelemetryRecord last_record = telemetry_buffer[last_record_index];
    
    mutex_exit(&telemetry_mutex);
    
    return last_record.to_csv();
}

std::string get_last_sensor_record_csv() {
    mutex_enter_blocking(&telemetry_mutex);

    if (telemetry_buffer_count == 0) {
        mutex_exit(&telemetry_mutex);
        return ""; 
    }

    size_t last_record_index = (telemetry_buffer_write_index + TELEMETRY_BUFFER_SIZE - 1) % TELEMETRY_BUFFER_SIZE;

    SensorDataRecord last_record = sensor_data_buffer[last_record_index];

    mutex_exit(&telemetry_mutex);

    return last_record.to_csv();
}