#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <string>
#include "pico/sync.h" // Include for mutex_t

class GPSData {
public:
    GPSData(); // Constructor to initialize the mutex

    std::string getNMEAData() {
        mutex_enter_blocking(&nmea_mutex_); // Wait indefinitely for the mutex
        std::string data = last_nmea_message_;
        mutex_exit(&nmea_mutex_); // Release the mutex
        return data;
    }

    void updateNMEAData(const std::string& data) {
        mutex_enter_blocking(&nmea_mutex_); // Wait indefinitely for the mutex
        last_nmea_message_ = data;
        mutex_exit(&nmea_mutex_); // Release the mutex
    }

private:
    std::string last_nmea_message_;
    mutex_t nmea_mutex_;
};

extern GPSData gps_data; // Declare the global instance
void collectGPSData();

#endif