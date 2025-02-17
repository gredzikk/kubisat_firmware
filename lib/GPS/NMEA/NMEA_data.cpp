#include "lib/GPS/NMEA/NMEA_data.h"

NMEAData nmea_data; // Define the global instance

NMEAData::NMEAData() {
    mutex_init(&rmc_mutex);
    mutex_init(&gga_mutex);
}

void NMEAData::updateRmcTokens(const std::vector<std::string>& tokens) {
    mutex_enter_blocking(&rmc_mutex);
    rmcTokens = tokens;
    mutex_exit(&rmc_mutex);
}

void NMEAData::updateGgaTokens(const std::vector<std::string>& tokens) {
    mutex_enter_blocking(&gga_mutex);
    ggaTokens = tokens;
    mutex_exit(&gga_mutex);
}

std::vector<std::string> NMEAData::getRmcTokens() const {
    mutex_enter_blocking(const_cast<mutex_t*>(&rmc_mutex));
    std::vector<std::string> copy = rmcTokens;
    mutex_exit(const_cast<mutex_t*>(&rmc_mutex));
    return copy;
}

std::vector<std::string> NMEAData::getGgaTokens() const {
    mutex_enter_blocking(const_cast<mutex_t*>(&gga_mutex));
    std::vector<std::string> copy = ggaTokens;
    mutex_exit(const_cast<mutex_t*>(&gga_mutex));
    return copy;
}