#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"
#include <ctime>

class NMEAData {
private:
    static NMEAData* instance;
    static mutex_t instance_mutex;

    std::vector<std::string> rmc_tokens_;
    std::vector<std::string> gga_tokens_;
    mutex_t rmc_mutex_;
    mutex_t gga_mutex_;

    // Private constructor
    NMEAData();
    
    // Delete copy constructor and assignment operator
    NMEAData(const NMEAData&) = delete;
    NMEAData& operator=(const NMEAData&) = delete;

public:
    static NMEAData& get_instance();

    void update_rmc_tokens(const std::vector<std::string>& tokens);
    void update_gga_tokens(const std::vector<std::string>& tokens);
    std::vector<std::string> get_rmc_tokens() const;
    std::vector<std::string> get_gga_tokens() const;
    bool has_valid_time() const;
    time_t get_unix_time() const;
};

#endif