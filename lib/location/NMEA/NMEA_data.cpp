#include "lib/location/NMEA/NMEA_data.h"

NMEAData nmea_data; // Define the global instance

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