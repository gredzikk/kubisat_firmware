// filepath: /c:/Users/Kuba/Desktop/inz/kubisat/software/kubisat_firmware/lib/GPS/nmea_data.h
#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"

class NMEAData {
public:
    NMEAData();
    void update_rmc_tokens(const std::vector<std::string>& tokens);
    void update_gga_tokens(const std::vector<std::string>& tokens);

    std::vector<std::string> get_rmc_tokens() const;
    std::vector<std::string> get_gga_tokens() const;

private:
    std::vector<std::string> rmc_tokens_;
    std::vector<std::string> gga_tokens_;
    mutex_t rmc_mutex_;
    mutex_t gga_mutex_;
};

extern NMEAData nmea_data;

#endif