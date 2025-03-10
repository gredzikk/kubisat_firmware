#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"
#include <ctime>

class NMEAData {
private:
    std::vector<std::string> rmc_tokens_;
    std::vector<std::string> gga_tokens_;
    mutex_t rmc_mutex_;
    mutex_t gga_mutex_;

    NMEAData() {
        mutex_init(&rmc_mutex_);
        mutex_init(&gga_mutex_);
    }

    NMEAData(const NMEAData&) = delete;
    NMEAData& operator=(const NMEAData&) = delete;

public:
    static NMEAData& get_instance() {
        static NMEAData instance;
        return instance;
    }

    void update_rmc_tokens(const std::vector<std::string>& tokens) {
        mutex_enter_blocking(&rmc_mutex_);
        rmc_tokens_ = tokens;
        mutex_exit(&rmc_mutex_);
    }

    void update_gga_tokens(const std::vector<std::string>& tokens) {
        mutex_enter_blocking(&gga_mutex_);
        gga_tokens_ = tokens;
        mutex_exit(&gga_mutex_);
    }

    std::vector<std::string> get_rmc_tokens() const {
        mutex_enter_blocking(const_cast<mutex_t*>(&rmc_mutex_));
        std::vector<std::string> copy = rmc_tokens_;
        mutex_exit(const_cast<mutex_t*>(&rmc_mutex_));
        return copy;
    }

    std::vector<std::string> get_gga_tokens() const {
        mutex_enter_blocking(const_cast<mutex_t*>(&gga_mutex_));
        std::vector<std::string> copy = gga_tokens_;
        mutex_exit(const_cast<mutex_t*>(&gga_mutex_));
        return copy;
    }

    bool has_valid_time() const {
        return rmc_tokens_.size() >= 10 && rmc_tokens_[1].length() > 5;
    }

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

        struct tm timeinfo = { 0 };

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