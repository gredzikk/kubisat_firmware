#include "lib/GPS/gps_collector.h"
#include "utils.h" // For uartPrint
#include "pico/time.h" // For time-related functions
#include "lib/GPS/NMEA/nmea_data.h"
#include "lib/GPS/NMEA/NMEA_parser.h" // Explicitly include NMEA_parser.h

#define MAX_RAW_DATA_LENGTH 1024

extern NMEAData nmea_data; // Access the global instance

void collectGPSData() {
    static char raw_data_buffer[MAX_RAW_DATA_LENGTH];
    static int raw_data_index = 0;
    static absolute_time_t last_print_time = {0}; // Initialize to 0

    while (uart_is_readable(GPS_UART_PORT)) {
        char c = uart_getc(GPS_UART_PORT);

        if (raw_data_index < MAX_RAW_DATA_LENGTH - 1) {
            raw_data_buffer[raw_data_index++] = c;
        } else {
            // Raw data buffer overflow
            uartPrint("Raw GPS data buffer overflow!");
            raw_data_index = 0; // Reset the index, discarding the oldest data
        }
    }

    // Process the raw data every second
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_print_time, now) >= 1000000) { // 1 second = 1,000,000 microseconds
        if (raw_data_index > 0) {
            raw_data_buffer[raw_data_index] = '\0'; // Null-terminate the string
            std::string raw_data_string(raw_data_buffer);
            uartPrint("Raw GPS data: " + raw_data_string);

            // Update the raw NMEA data in the global NMEAData instance
            nmea_data.updateNMEAData(raw_data_string);

            // Attempt to parse the NMEA data
            try {
                RMCMessage* rmcMessage = parseNMEA(raw_data_string);
                if (rmcMessage) {
                    // Extract relevant data and update the parsed data structure
                    ParsedGPSData parsedData;
                    parsedData.time = rmcMessage->time;
                    parsedData.latitude = rmcMessage->latitude;
                    parsedData.latitudeDirection = rmcMessage->latitudeDirection;
                    parsedData.longitude = rmcMessage->longitude;
                    parsedData.longitudeDirection = rmcMessage->longitudeDirection;
                    parsedData.speedOverGround = rmcMessage->speedOverGround;
                    parsedData.courseOverGround = rmcMessage->courseOverGround; // 09 09 09 09 fFff
                    parsedData.date = rmcMessage->date;

                    // Update the parsed data in the global NMEAData instance
                    nmea_data.updateParsedData(parsedData);
                    EventManager::emit(EventGroup::GPS, GPSEvent::DATA_READY);
                    rmcMessage->print();
                    delete rmcMessage;
                }
            } catch (const std::exception& e) {
                uartPrint("NMEA Parsing Error: " + std::string(e.what()));
            }

            raw_data_index = 0; // Reset the index after processing
        }
        last_print_time = now; // Update the last print time
    }
}