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
  * @brief Save buffered telemetry data to storage
  * @return True if data was successfully saved
  * @details Writes all records from the telemetry buffer to the CSV file
  *          and clears the buffer after successful writing
  */
 bool flush_telemetry();
 
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
  * @brief Get the current sample interval in milliseconds
  * @return Sample interval in milliseconds
  */
 uint32_t get_telemetry_sample_interval();
 
 /**
  * @brief Set the telemetry sample interval
  * @param interval_ms New interval in milliseconds
  * @details Sets a minimum bound of 100ms to prevent excessive sampling
  */
 void set_telemetry_sample_interval(uint32_t interval_ms);
 
 /**
  * @brief Get the number of records before flushing to storage
  * @return Number of records in flush threshold
  */
 uint32_t get_telemetry_flush_threshold();
 
 /**
  * @brief Set the number of records before flushing to storage
  * @param records Number of records in flush threshold
  * @details Sets reasonable bounds (1-100) for the threshold value
  */
 void set_telemetry_flush_threshold(uint32_t records);
 
 #endif // TELEMETRY_MANAGER_H
 
 /** @} */ // End of TelemetryManager group