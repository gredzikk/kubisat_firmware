// filepath: /c:/Users/Kuba/Desktop/inz/kubisat/software/kubisat_firmware/lib/GPS/gps_collector.cpp
#include "lib/location/gps_collector.h"
#include "utils.h"
#include "pico/time.h"
#include "lib/location/NMEA/nmea_data.h"
#include "event_manager.h"
#include <vector>
#include <ctime>
#include <cstring>
#include "DS3231.h"
#include <sstream>

#define MAX_RAW_DATA_LENGTH 1024

extern NMEAData nmea_data;

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void collectGPSData() {
    static char raw_data_buffer[MAX_RAW_DATA_LENGTH];
    static int raw_data_index = 0;

    while (uart_is_readable(GPS_UART_PORT)) {
        char c = uart_getc(GPS_UART_PORT);

        if (c == '\r' || c == '\n') {
            // End of message
            if (raw_data_index > 0) {
                raw_data_buffer[raw_data_index] = '\0';
                std::string message(raw_data_buffer);
                raw_data_index = 0;

                // Split the message into tokens
                std::vector<std::string> tokens = splitString(message, ',');

                // Update the global vectors based on the sentence type
                if (message.find("$GPRMC") == 0) {
                    nmea_data.updateRmcTokens(tokens);
                } else if (message.find("$GPGGA") == 0) {
                    nmea_data.updateGgaTokens(tokens);
                }
            }
        } else {
            // Append to buffer
            if (raw_data_index < MAX_RAW_DATA_LENGTH - 1) {
                raw_data_buffer[raw_data_index++] = c;
            } else {
                raw_data_index = 0;
            }
        }
    }
}