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

extern PowerManager powerManager;
extern DS3231 systemClock;
extern NMEAData nmea_data; 

#define TELEMETRY_CSV_PATH "/telemetry.csv"
#define DEFAULT_SAMPLE_INTERVAL_MS 2000
#define DEFAULT_FLUSH_THRESHOLD 10

static uint32_t sample_interval_ms = DEFAULT_SAMPLE_INTERVAL_MS;
static uint32_t flush_threshold = DEFAULT_FLUSH_THRESHOLD;

struct TelemetryRecord {
    uint32_t timestamp;
    
    // Power data
    float battery_voltage;
    float system_voltage;
    float charge_current_usb;
    float charge_current_solar;
    float discharge_current;
    
    // Environmental sensor data
    float temperature;
    float pressure;
    float humidity;
    float light_level;
    
    // GPS data - key RMC fields
    std::string time;         // UTC time
    std::string latitude;     // Latitude
    std::string lat_dir;      // N/S
    std::string longitude;    // Longitude
    std::string lon_dir;      // E/W
    std::string speed;        // Speed in knots
    std::string course;       // Course in degrees
    std::string date;         // Date
    
    // GPS data - key GGA fields
    std::string fix_quality;  // Fix quality
    std::string satellites;   // Number of satellites
    std::string altitude;     // Altitude
    
    // Convert record to CSV line
    std::string to_csv() const {
        std::stringstream ss;
        ss << timestamp << "," 
           << std::fixed << std::setprecision(3)
           << battery_voltage << ","
           << system_voltage << ","
           << charge_current_usb << ","
           << charge_current_solar << ","
           << discharge_current << ","
           << temperature << ","
           << pressure << ","
           << humidity << ","
           << light_level << ","
           // GPS RMC data
           << time << ","
           << latitude << "," << lat_dir << ","
           << longitude << "," << lon_dir << ","
           << speed << ","
           << course << ","
           << date << ","
           // GPS GGA data
           << fix_quality << ","
           << satellites << ","
           << altitude;
        return ss.str();
    }
};

// Circular buffer for telemetry records
static std::deque<TelemetryRecord> telemetry_buffer;
static mutex_t telemetry_mutex;

bool telemetry_init() {
    mutex_init(&telemetry_mutex);
    
    // Create CSV file with headers if it doesn't exist
    if (sd_card_mounted) {
        FILE* file = fopen(TELEMETRY_CSV_PATH, "r");
        if (!file) {
            file = fopen(TELEMETRY_CSV_PATH, "w");
            if (file) {
                fprintf(file, "timestamp,battery_v,system_v,usb_ma,solar_ma,discharge_ma,temp_c,press_hpa,humidity_pct,light_lux,"
                              "gps_time,latitude,lat_dir,longitude,lon_dir,speed_knots,course_deg,date,"
                              "fix_quality,satellites,altitude_m\n");
                fclose(file);
                uart_print("Created new telemetry log", VerbosityLevel::INFO);
                return true;
            } else {
                uart_print("Failed to create telemetry log", VerbosityLevel::ERROR);
                return false;
            }
        } else {
            fclose(file);
            return true;
        }
    }
    
    uart_print("Telemetry system initialized (storage not available)", VerbosityLevel::WARNING);
    return false;
}

bool collect_telemetry() {
    SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    
    TelemetryRecord record;
    record.timestamp = systemClock.get_unix_time();
    
    // Power data
    record.battery_voltage = powerManager.get_voltage_battery();
    record.system_voltage = powerManager.get_voltage_5v();
    record.charge_current_usb = powerManager.get_current_charge_usb();
    record.charge_current_solar = powerManager.get_current_charge_solar();
    record.discharge_current = powerManager.get_current_draw();
    
    // Sensor data - check if sensors are available first
    record.temperature = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::TEMPERATURE);
    record.pressure = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::PRESSURE);
    record.humidity = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::HUMIDITY);
    record.light_level = sensor_wrapper.sensor_read_data(SensorType::LIGHT, SensorDataTypeIdentifier::LIGHT_LEVEL);
    
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
    
    // Add to buffer with mutex protection
    mutex_enter_blocking(&telemetry_mutex);
    
    telemetry_buffer.push_back(record);
    if (telemetry_buffer.size() > flush_threshold) {
        telemetry_buffer.pop_front(); // Keep buffer size limited
    }
    
    mutex_exit(&telemetry_mutex);
    return true;
}

bool flush_telemetry() {
    if (!sd_card_mounted) {
        bool status = fs_init();
        if (!status) {
            uart_print("Failed to mount storage for telemetry flush", VerbosityLevel::ERROR);
            return false;
        }
    }
    
    mutex_enter_blocking(&telemetry_mutex);
    
    if (telemetry_buffer.empty()) {
        mutex_exit(&telemetry_mutex);
        return true; // Nothing to save
    }
    
    FILE* file = fopen(TELEMETRY_CSV_PATH, "a");
    if (!file) {
        uart_print("Failed to open telemetry log for writing", VerbosityLevel::ERROR);
        mutex_exit(&telemetry_mutex);
        return false;
    }
    
    // Write all records to CSV
    for (const auto& record : telemetry_buffer) {
        fprintf(file, "%s\n", record.to_csv().c_str());
    }
    
    // Clear buffer after successful write
    telemetry_buffer.clear();
    
    fclose(file);
    
    mutex_exit(&telemetry_mutex);
    
    uart_print("Flushed telemetry data to storage", VerbosityLevel::DEBUG);
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
    if (records >= 1 && records <= 100) { // Reasonable limits
        flush_threshold = records;
    }
}