#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <string>
#include "lib/GPS/NMEA/nmea_data.h"
#include <iostream> // Add this include
#include <vector>   // Add this include
#include <sstream>  // Add this include
#include <iomanip>  // Add this include

// Helper function to split a string by a delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter);

// Function to calculate the NMEA checksum
std::string calculateChecksum(const std::string& sentence);

// RMC - Recommended Minimum Navigation Information
class RMCMessage {
public:
    std::string time;
    char status;
    double latitude;
    char latitudeDirection;
    double longitude;
    char longitudeDirection;
    double speedOverGround;
    double courseOverGround;
    std::string date;
    double magneticVariation;
    char magneticVariationDirection;

    void parse(const std::string& message);

    void print() const;
};

// Function to parse an NMEA message
RMCMessage* parseNMEA(const std::string& message);

#endif