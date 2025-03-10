/**
 * @file NMEA_data.h
 * @brief Header file for the NMEAData class, which manages parsed NMEA sentences.
 *
 * @details This file defines the NMEAData class, a singleton that stores and
 *          provides access to parsed data from NMEA sentences received from a GPS module.
 *          It includes methods for updating and retrieving RMC and GGA tokens,
 *          as well as converting the data to a Unix timestamp.
 *
 * @defgroup Location Location
 * @brief Classes for handling location data.
 *
 * @{
 */

#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"
#include <ctime>
#include <cstring>

/**
 * @brief Manages parsed NMEA sentences.
 * @details This class is a singleton that stores and provides access to parsed
 *          data from NMEA sentences received from a GPS module. It includes
 *          methods for updating and retrieving RMC and GGA tokens, as well as
 *          converting the data to a Unix timestamp.
 * @ingroup Location
 */
class NMEAData {
private:
    /** @brief Vector of tokens from the most recent RMC sentence. */
    std::vector<std::string> rmc_tokens_;
    /** @brief Vector of tokens from the most recent GGA sentence. */
    std::vector<std::string> gga_tokens_;
    /** @brief Mutex for thread-safe access to the RMC tokens. */
    mutex_t rmc_mutex_;
    /** @brief Mutex for thread-safe access to the GGA tokens. */
    mutex_t gga_mutex_;

    /**
     * @brief Private constructor for the singleton pattern.
     * @details Initializes the mutexes.
     */
    NMEAData() {
        mutex_init(&rmc_mutex_);
        mutex_init(&gga_mutex_);
    }

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    NMEAData(const NMEAData&) = delete;
    /**
     * @brief Deleted assignment operator to prevent assignment.
     */
    NMEAData& operator=(const NMEAData&) = delete;

public:
    /**
     * @brief Gets the singleton instance of the NMEAData class.
     * @return A reference to the singleton instance.
     */
    static NMEAData& get_instance() {
        static NMEAData instance;
        return instance;
    }

    /**
     * @brief Updates the RMC tokens with new data.
     * @param[in] tokens Vector of strings representing the RMC tokens.
     */
    void update_rmc_tokens(const std::vector<std::string>& tokens) {
        mutex_enter_blocking(&rmc_mutex_);
        rmc_tokens_ = tokens;
        mutex_exit(&rmc_mutex_);
    }

    /**
     * @brief Updates the GGA tokens with new data.
     * @param[in] tokens Vector of strings representing the GGA tokens.
     */
    void update_gga_tokens(const std::vector<std::string>& tokens) {
        mutex_enter_blocking(&gga_mutex_);
        gga_tokens_ = tokens;
        mutex_exit(&gga_mutex_);
    }

    /**
     * @brief Gets a copy of the RMC tokens.
     * @return A copy of the RMC tokens.
     */
    std::vector<std::string> get_rmc_tokens() const {
        mutex_enter_blocking(const_cast<mutex_t*>(&rmc_mutex_));
        std::vector<std::string> copy = rmc_tokens_;
        mutex_exit(const_cast<mutex_t*>(&rmc_mutex_));
        return copy;
    }

    /**
     * @brief Gets a copy of the GGA tokens.
     * @return A copy of the GGA tokens.
     */
    std::vector<std::string> get_gga_tokens() const {
        mutex_enter_blocking(const_cast<mutex_t*>(&gga_mutex_));
        std::vector<std::string> copy = gga_tokens_;
        mutex_exit(const_cast<mutex_t*>(&gga_mutex_));
        return copy;
    }

    /**
     * @brief Checks if the NMEA data has valid time information.
     * @return True if the data has valid time information, false otherwise.
     */
    bool has_valid_time() const {
        return rmc_tokens_.size() >= 10 && rmc_tokens_[1].length() > 5;
    }

    /**
     * @brief Converts the NMEA data to a Unix timestamp.
     * @return The Unix timestamp, or 0 if the data is invalid.
     */
    time_t get_unix_time() const {
        if (!has_valid_time()) {
            return 0; // Invalid time
        }

        // Parse date and time from RMC tokens
        // Format: hhmmss.sss,A,ddmm.mmmm,N,dddmm.mmmm,W,speed,course,ddmmyy
        std::string time_str = rmc_tokens_[1]; // hhmmss.sss
        std::string date_str = rmc_tokens_[9]; // ddmmyy

        if (time_str.length() < 6 || date_str.length() < 6) {
            return 0;
        }

        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(timeinfo));

        // Parse time: hours (0-1), minutes (2-3), seconds (4-5)
        timeinfo.tm_hour = std::stoi(time_str.substr(0, 2));
        timeinfo.tm_min = std::stoi(time_str.substr(2, 2));
        timeinfo.tm_sec = std::stoi(time_str.substr(4, 2));

        // Parse date: day (0-1), month (2-3), year (4-5)
        timeinfo.tm_mday = std::stoi(date_str.substr(0, 2));
        timeinfo.tm_mon = std::stoi(date_str.substr(2, 2)) - 1; // Month is 0-11
        timeinfo.tm_year = std::stoi(date_str.substr(4, 2)) + 100; // Year is since 1900

        return mktime(&timeinfo);
    }
};

#endif
/** @} */