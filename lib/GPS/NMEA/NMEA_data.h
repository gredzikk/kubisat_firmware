// filepath: /c:/Users/Kuba/Desktop/inz/kubisat/software/kubisat_firmware/lib/GPS/nmea_data.h
#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"

class NMEAData {
public:
    NMEAData();
    void updateRmcTokens(const std::vector<std::string>& tokens);
    void updateGgaTokens(const std::vector<std::string>& tokens);

    std::vector<std::string> getRmcTokens() const;
    std::vector<std::string> getGgaTokens() const;

private:
    std::vector<std::string> rmcTokens;
    std::vector<std::string> ggaTokens;
    mutex_t rmc_mutex;
    mutex_t gga_mutex;
};

extern NMEAData nmea_data;

#endif