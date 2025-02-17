#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <string>
#include "pico/sync.h" // Include for mutex_t

// Structure to hold parsed GPS data
struct ParsedGPSData {
    std::string time;
    double latitude;
    char latitudeDirection;
    double longitude;
    char longitudeDirection;
    double speedOverGround;
    double courseOverGround;
    std::string date;
};

class NMEAData {
public:
    NMEAData(); // Constructor to initialize the mutex

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

    // Method to update the parsed GPS data
    void updateParsedData(const ParsedGPSData& data) {
        mutex_enter_blocking(&parsed_data_mutex_);
        parsed_data_ = data;
        mutex_exit(&parsed_data_mutex_);
    }

    // Method to retrieve the parsed GPS data
    ParsedGPSData getParsedData() {
        mutex_enter_blocking(&parsed_data_mutex_);
        ParsedGPSData data = parsed_data_;
        mutex_exit(&parsed_data_mutex_);
        return data;
    }

private:
    std::string last_nmea_message_;
    mutex_t nmea_mutex_;

    ParsedGPSData parsed_data_;
    mutex_t parsed_data_mutex_;
};

extern NMEAData nmea_data; // Declare the global instance

#endif