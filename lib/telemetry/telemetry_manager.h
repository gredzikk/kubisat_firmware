/**
 * @file telemetry_manager.h
 * @brief System telemetry collection and logging
 * @details This module handles periodic collection and storage of telemetry
 *          data from various satellite subsystems including power management,
 *          sensors (temperature, pressure, humidity, light), and GPS data.
 *          
 *          Telemetry is collected at configurable intervals and stored in a
 *          circular buffer before being flushed to persistent storage after
 *          a configurable number of records are collected.
 * 
 * @defgroup TelemetryManager Telemetry Manager
 * @{
 */


#ifndef TELEMETRY_MANAGER_H
#define TELEMETRY_MANAGER_H

#include <cstdint>
#include <string>
#include "pico/stdlib.h"
#include "lib/location/NMEA/nmea_data.h"
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
#include <functional>

/**
* @struct TelemetryRecord
* @brief Structure representing a single telemetry data point
* @details Contains all measurements from power subsystem, sensors, and GPS data
*          collected at a specific point in time
*/
struct TelemetryRecord {
    uint32_t timestamp;       /**< Unix timestamp of the record */

    std::string build_version; /**< Build version of the firmware */
    
    // Power data
    float battery_voltage;    /**< Battery voltage in volts */
    float system_voltage;     /**< System 5V rail voltage in volts */
    float charge_current_usb; /**< USB charging current in mA */
    float charge_current_solar; /**< Solar charging current in mA */
    float discharge_current;  /**< Battery discharge current in mA */
    
    // GPS data - key RMC fields
    std::string time;         /**< UTC time from GPS */
    std::string latitude;     /**< Latitude from GPS */
    std::string lat_dir;      /**< N/S latitude direction */
    std::string longitude;    /**< Longitude from GPS */
    std::string lon_dir;      /**< E/W longitude direction */
    std::string speed;        /**< Speed in knots */
    std::string course;       /**< Course in degrees */
    std::string date;         /**< Date from GPS */
    
    // GPS data - key GGA fields
    std::string fix_quality;  /**< GPS fix quality */
    std::string satellites;   /**< Number of satellites in view */
    std::string altitude;     /**< Altitude in meters */
    
    
    /**
     * @brief Converts the telemetry record to a CSV string.
     * @return A CSV string representing the telemetry record.
     * @ingroup TelemetryManager
     */
    std::string to_csv() const {
        std::stringstream ss;
        ss << timestamp << "," 
           << build_version << ","
           << std::fixed << std::setprecision(3)
           << battery_voltage << ","
           << system_voltage << ","
           << charge_current_usb << ","
           << charge_current_solar << ","
           << discharge_current << ","

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


/**
 * @struct SensorDataRecord
 * @brief Structure representing a single sensor data point
 * @details Contains measurements from the environment and light sensors
 *          collected at a specific point in time
 * @ingroup TelemetryManager
 */
struct SensorDataRecord {
    uint32_t timestamp;       /**< Unix timestamp of the record */
    float temperature;        /**< Temperature in degrees Celsius */
    float pressure;           /**< Pressure in hPa */
    float humidity;           /**< Relative humidity in % */
    float light;              /**< Light intensity in lux */
    
    /**
     * @brief Converts the sensor data record to a CSV string.
     * @return A CSV string representing the sensor data record.
     * @ingroup TelemetryManager
     */
    std::string to_csv() const {
        std::stringstream ss;
        ss << timestamp << "," 
           << std::fixed << std::setprecision(3)
           << temperature << ","
           << pressure << ","
           << humidity << ","
           << light;
        return ss.str();
    }
};

/**
 * @brief Circular buffer for telemetry records
 */
constexpr int TELEMETRY_BUFFER_SIZE = 20;
extern const int TELEMETRY_BUFFER_SIZE;
extern TelemetryRecord telemetry_buffer[TELEMETRY_BUFFER_SIZE];
extern size_t telemetry_buffer_count;
extern size_t telemetry_buffer_write_index;

extern SensorDataRecord sensor_data_buffer[TELEMETRY_BUFFER_SIZE];
extern size_t sensor_data_buffer_count;
extern size_t sensor_data_buffer_write_index;

/**
 * @brief Mutex for thread-safe access to the telemetry buffer
 */
extern mutex_t telemetry_mutex;


/**
 * @brief Initialize the telemetry system
 * @return True if initialization was successful
 * @details Sets up the mutex for thread-safe buffer access and creates a telemetry
 *          CSV file with appropriate headers if it doesn't already exist
 */
bool telemetry_init();

/**
 * @brief Collect telemetry data from sensors and power subsystems
 * @return True if data was successfully collected
 * @details Reads data from power manager, sensors, and GPS and stores it
 *          in the telemetry buffer with proper mutex protection
 */
bool collect_telemetry();

/**
 * @brief Collects power subsystem telemetry data.
 * @param[out] record The telemetry record to update with power data.
 * @ingroup TelemetryManager
 */
void collect_power_telemetry(TelemetryRecord& record);

/**
 * @brief Emits power-related events based on current and voltage levels.
 * @param[in] battery_voltage The current battery voltage.
 * @param[in] charge_current_usb The current USB charging current.
 * @param[in] charge_current_solar The current solar charging current.
 * @ingroup TelemetryManager
 */
void emit_power_events(float battery_voltage, float charge_current_usb, float charge_current_solar);

/**
 * @brief Collects GPS telemetry data.
 * @param[out] record The telemetry record to update with GPS data.
 * @ingroup TelemetryManager
 */
void collect_gps_telemetry(TelemetryRecord& record);

/**
 * @brief Collects sensor telemetry data.
 * @param[out] sensor_record The sensor data record to update with sensor data.
 * @ingroup TelemetryManager
 */
void collect_sensor_telemetry(SensorDataRecord& sensor_record);

/**
 * @brief Save buffered telemetry data to storage
 * @return True if data was successfully saved
 * @details Writes all records from the telemetry buffer to the CSV file
 *          and clears the buffer after successful writing
 */
bool flush_telemetry();

/**
* @brief Save buffered sensor data to storage
* @return True if data was successfully saved
* @details Writes all records from the sensor data buffer to the CSV file
*          and clears the buffer after successful writing
*/
bool flush_sensor_data();

/**
 * @brief Check if it's time to collect telemetry based on interval
 * @param current_time Current system time in milliseconds
 * @param last_collection_time Previous collection time in milliseconds
 * @return True if collection interval has passed
 * @details Updates last_collection_time if the interval has passed
 */
bool is_telemetry_collection_time(uint32_t current_time, uint32_t& last_collection_time);

/**
 * @brief Check if it's time to flush telemetry buffer based on count
 * @param collection_counter Current collection counter
 * @return True if flush threshold has been reached
 * @details Resets collection_counter to zero if the threshold has been reached
 */
bool is_telemetry_flush_time(uint32_t& collection_counter);


/**
 * @brief Gets the last telemetry record as a CSV string.
 * @return A CSV string representing the last telemetry record, or an empty string if no data is available.
 */
std::string get_last_telemetry_record_csv();

/**
 * @brief Gets the last sensor data record as a CSV string.
 * @return A CSV string representing the last sensor data record, or an empty string if no data is available.
 */
std::string get_last_sensor_record_csv();

#endif // TELEMETRY_MANAGER_H

 /** @} */ // End of TelemetryManager group