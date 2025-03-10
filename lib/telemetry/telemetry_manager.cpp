/**
 * @file telemetry_manager.cpp
 * @brief Implementation of telemetry collection and storage functionality
 * @details Handles collecting, buffering, and persisting telemetry data from various
 *          satellite subsystems including power, sensors, and GPS
 * @defgroup TelemetryManager Telemetry Manager
 * @{
 */

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

/**
 * @brief Initialize the telemetry system
 * @return True if initialization was successful
 * @details Sets up the mutex for thread-safe buffer access and creates a telemetry
 *          CSV file with appropriate headers if it doesn't already exist
 * @ingroup TelemetryManager
 */
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


/**
 * @brief Collects power subsystem telemetry data.
 * @param[out] record The telemetry record to update with power data.
 * @ingroup TelemetryManager
 */
void collect_power_telemetry(TelemetryRecord& record) {
    record.battery_voltage = PowerManager::get_instance().get_voltage_battery();
    record.system_voltage = PowerManager::get_instance().get_voltage_5v();
    record.charge_current_usb = PowerManager::get_instance().get_current_charge_usb();
    record.charge_current_solar = PowerManager::get_instance().get_current_charge_solar();
    record.discharge_current = PowerManager::get_instance().get_current_draw();
}

/**
 * @brief Emits power-related events based on current and voltage levels.
 * @param[in] battery_voltage The current battery voltage.
 * @param[in] charge_current_usb The current USB charging current.
 * @param[in] charge_current_solar The current solar charging current.
 * @ingroup TelemetryManager
 */
void emit_power_events(float battery_voltage, float charge_current_usb, float charge_current_solar) {
    static bool usb_charging_active = false;
    static bool solar_charging_active = false;
    static bool battery_low = false;
    static bool battery_full = false;

    if (charge_current_usb > PowerManager::USB_CURRENT_THRESHOLD && !usb_charging_active) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_CONNECTED);
        usb_charging_active = true;
    } else if (charge_current_usb < PowerManager::USB_CURRENT_THRESHOLD && usb_charging_active) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_DISCONNECTED);
        usb_charging_active = false;
    }

    if (charge_current_solar > PowerManager::SOLAR_CURRENT_THRESHOLD && !solar_charging_active) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_ACTIVE);
        solar_charging_active = true;
    } else if (charge_current_solar < PowerManager::SOLAR_CURRENT_THRESHOLD && solar_charging_active) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_INACTIVE);
        solar_charging_active = false;
    }

    if (battery_voltage < PowerManager::BATTERY_LOW_THRESHOLD && !battery_low) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_LOW);
        battery_low = true;
        battery_full = false; // Cancel overcharge event
    } else if (battery_voltage > PowerManager::BATTERY_FULL_THRESHOLD && !battery_full) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_FULL);
        battery_full = true;
        battery_low = false; // Cancel low battery event
    } else if (battery_voltage > PowerManager::BATTERY_LOW_THRESHOLD && battery_low) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_NORMAL);
        battery_low = false;
    } else if (battery_voltage < PowerManager::BATTERY_FULL_THRESHOLD && battery_full) {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_NORMAL);
        battery_full = false;
    }
}

/**
 * @brief Collects GPS telemetry data.
 * @param[out] record The telemetry record to update with GPS data.
 * @ingroup TelemetryManager
 */
void collect_gps_telemetry(TelemetryRecord& record) {
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
}

/**
 * @brief Collects sensor telemetry data.
 * @param[out] sensor_record The sensor data record to update with sensor data.
 * @ingroup TelemetryManager
 */
void collect_sensor_telemetry(SensorDataRecord& sensor_record) {
    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    sensor_record.temperature = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::TEMPERATURE);
    sensor_record.pressure = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::PRESSURE);
    sensor_record.humidity = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::HUMIDITY);
    sensor_record.light = sensor_wrapper.sensor_read_data(SensorType::LIGHT, SensorDataTypeIdentifier::LIGHT_LEVEL);
}

/**
 * @brief Collect telemetry data from sensors and power subsystems
 * @return True if data was successfully collected
 * @details Reads data from power manager, sensors, and GPS and stores it
 *          in the telemetry buffer with proper mutex protection
 * @ingroup TelemetryManager
 */
bool collect_telemetry() {
    uint32_t timestamp = DS3231::get_instance().get_local_time();
    TelemetryRecord record;
    record.timestamp = timestamp;
    record.build_version = std::to_string(BUILD_NUMBER);

    // Collect power telemetry and emit events
    collect_power_telemetry(record);
    emit_power_events(record.battery_voltage, record.charge_current_usb, record.charge_current_solar);

    // Collect GPS telemetry
    collect_gps_telemetry(record);

    // Collect sensor telemetry
    SensorDataRecord sensor_record;
    sensor_record.timestamp = timestamp;
    collect_sensor_telemetry(sensor_record);

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


/**
 * @brief Save buffered telemetry data to storage
 * @return True if data was successfully saved
 * @details Writes all records from the telemetry buffer to the CSV file
 *          and clears the buffer after successful writing
 * @ingroup TelemetryManager
 */
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


/**
 * @brief Save buffered sensor data to storage
 * @return True if data was successfully saved
 * @details Writes all records from the sensor data buffer to the CSV file
 *          and clears the buffer after successful writing
 * @ingroup TelemetryManager
 */
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


/**
 * @brief Check if it's time to collect telemetry based on interval
 * @param current_time Current system time in milliseconds
 * @param last_collection_time Previous collection time in milliseconds
 * @return True if collection interval has passed
 * @details Updates last_collection_time if the interval has passed
 * @ingroup TelemetryManager
 */
bool is_telemetry_collection_time(uint32_t current_time, uint32_t& last_collection_time) {
    if (current_time - last_collection_time >= sample_interval_ms) {
        last_collection_time = current_time;
        return true;
    }
    return false;
}


/**
 * @brief Check if it's time to flush telemetry buffer based on count
 * @param collection_counter Current collection counter
 * @return True if flush threshold has been reached
 * @details Resets collection_counter to zero if the threshold has been reached
 * @ingroup TelemetryManager
 */
bool is_telemetry_flush_time(uint32_t& collection_counter) {
    if (collection_counter >= flush_threshold) {
        collection_counter = 0;
        return true;
    }
    return false;
}

/**
 * @brief Gets the last telemetry record as a CSV string.
 * @return A CSV string representing the last telemetry record, or an empty string if no data is available.
 * @ingroup TelemetryManager
 */
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

/**
 * @brief Gets the last sensor data record as a CSV string.
 * @return A CSV string representing the last sensor data record, or an empty string if no data is available.
 * @ingroup TelemetryManager
 */
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