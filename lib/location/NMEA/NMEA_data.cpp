#include "lib/location/NMEA/NMEA_data.h"

NMEAData nmea_data; 

NMEAData::NMEAData() {
    mutex_init(&rmc_mutex_);
    mutex_init(&gga_mutex_);
}

void NMEAData::update_rmc_tokens(const std::vector<std::string>& tokens) {
    mutex_enter_blocking(&rmc_mutex_);
    rmc_tokens_ = tokens;
    mutex_exit(&rmc_mutex_);
}

void NMEAData::update_gga_tokens(const std::vector<std::string>& tokens) {
    mutex_enter_blocking(&gga_mutex_);
    gga_tokens_ = tokens;
    mutex_exit(&gga_mutex_);
}

std::vector<std::string> NMEAData::get_rmc_tokens() const {
    mutex_enter_blocking(const_cast<mutex_t*>(&rmc_mutex_));
    std::vector<std::string> copy = rmc_tokens_;
    mutex_exit(const_cast<mutex_t*>(&rmc_mutex_));
    return copy;
}

std::vector<std::string> NMEAData::get_gga_tokens() const {
    mutex_enter_blocking(const_cast<mutex_t*>(&gga_mutex_));
    std::vector<std::string> copy = gga_tokens_;
    mutex_exit(const_cast<mutex_t*>(&gga_mutex_));
    return copy;
}

bool NMEAData::has_valid_time() const {
    return rmc_tokens_.size() >= 10 && rmc_tokens_[1].length() > 5;
}

time_t NMEAData::get_unix_time() const {
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
    
    struct tm timeinfo = {0};
    
    // Parse time: hours (0-1), minutes (2-3), seconds (4-5)
    timeinfo.tm_hour = std::stoi(time_str.substr(0, 2));
    timeinfo.tm_min = std::stoi(time_str.substr(2, 2));
    timeinfo.tm_sec = std::stoi(time_str.substr(4, 2));
    
    // Parse date: day (0-1), month (2-3), year (4-5)
    timeinfo.tm_mday = std::stoi(date_str.substr(0, 2));
    timeinfo.tm_mon = std::stoi(date_str.substr(2, 2)) - 1; // Months from 0-11
    timeinfo.tm_year = std::stoi(date_str.substr(4, 2)) + 100; // Years since 1900
    
    // Convert to unix time
    return mktime(&timeinfo);
}