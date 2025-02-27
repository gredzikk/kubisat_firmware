#ifndef TELEMETRY_MANAGER_H
#define TELEMETRY_MANAGER_H

#include <cstdint>
#include <string>
#include "pico/stdlib.h"
#include "lib/location/NMEA/nmea_data.h"

/**
 * @file telemetry_manager.h
 * @brief System telemetry collection and logging
 * @details This module handles periodic collection and storage of telemetry
 *          data from various satellite subsystems
 */

/**
 * @brief Initialize the telemetry system
 * @return True if initialization was successful
 */
bool telemetry_init();

/**
 * @brief Collect telemetry data from sensors and power subsystems
 * @return True if data was successfully collected
 */
bool collect_telemetry();

/**
 * @brief Save buffered telemetry data to storage
 * @return True if data was successfully saved
 */
bool flush_telemetry();

/**
 * @brief Check if it's time to collect telemetry based on interval
 * @param current_time Current system time in milliseconds
 * @param last_collection_time Previous collection time in milliseconds
 * @return True if collection interval has passed
 */
bool is_telemetry_collection_time(uint32_t current_time, uint32_t& last_collection_time);

/**
 * @brief Check if it's time to flush telemetry buffer based on count
 * @param collection_counter Current collection counter
 * @return True if flush threshold has been reached
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
 */
void set_telemetry_flush_threshold(uint32_t records);

#endif // TELEMETRY_MANAGER_H