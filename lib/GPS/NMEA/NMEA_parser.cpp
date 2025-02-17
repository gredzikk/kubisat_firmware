#include "lib/GPS/NMEA/NMEA_parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Helper function to split a string by a delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to calculate the NMEA checksum
std::string calculateChecksum(const std::string& sentence) {
    unsigned char checksum = 0;
    for (char c : sentence) {
        checksum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(checksum);
    return ss.str();
}

void RMCMessage::parse(const std::string& message) {
    std::vector<std::string> tokens = splitString(message, ',');
    if (tokens.size() != 13) {
        throw std::runtime_error("Invalid RMC message format");
    }

    time = tokens[1];
    status = tokens[2][0];
    latitude = std::stod(tokens[3]);
    latitudeDirection = tokens[4][0];
    longitude = std::stod(tokens[5]);
    longitudeDirection = tokens[6][0];
    speedOverGround = std::stod(tokens[7]);
    courseOverGround = std::stod(tokens[8]);
    date = tokens[9];
    magneticVariation = (tokens[10].empty()) ? 0.0 : std::stod(tokens[10]);
    magneticVariationDirection = (tokens[11].empty()) ? ' ' : tokens[11][0];
}

void RMCMessage::print() const {
    std::cout << "RMC Message:\n";
    std::cout << "  Time: " << time << "\n";
    std::cout << "  Status: " << status << "\n";
    std::cout << "  Latitude: " << latitude << latitudeDirection << "\n";
    std::cout << "  Longitude: " << longitude << longitudeDirection << "\n";
    std::cout << "  Speed Over Ground: " << speedOverGround << "\n";
    std::cout << "  Course Over Ground: " << courseOverGround << "\n";
    std::cout << "  Date: " << date << "\n";
    std::cout << "  Magnetic Variation: " << magneticVariation << magneticVariationDirection << "\n";
}

// Function to parse an NMEA message
RMCMessage* parseNMEA(const std::string& message) {
    RMCMessage* rmcMessage = nullptr; // Initialize rmcMessage to nullptr

    // Check if the message is an RMC message
    if (message.find("$GPRMC") != 0) {
        return nullptr; // Or throw an exception: throw std::runtime_error("Not an RMC message");
    }

    // Remove the $ and checksum part
    size_t asteriskPos = message.find('*');
    if (asteriskPos == std::string::npos) {
        return nullptr; // Or throw an exception
    }
    std::string sentence = message.substr(1, asteriskPos - 1);

    // Verify checksum
    std::string expectedChecksum = message.substr(asteriskPos + 1);
    std::string calculatedChecksum = calculateChecksum(sentence);

    if (expectedChecksum != calculatedChecksum) {
        return nullptr; // Or throw an exception
    }

    rmcMessage = new RMCMessage();
    try {
        rmcMessage->parse(sentence);
    } catch (const std::exception& e) {
        delete rmcMessage;
        rmcMessage = nullptr;
        throw;
    }
    return rmcMessage;
}