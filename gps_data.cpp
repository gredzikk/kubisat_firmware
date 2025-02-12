// In a suitable file, e.g., communication.cpp or a new gps_handler.cpp
#include "gps_data.h"
#include "hardware/uart.h"
#include "utils.h" // For uartPrint
#include "pico/time.h" // For time-related functions

#define MAX_RAW_DATA_LENGTH 1024

GPSData gps_data; // Define the global instance

GPSData::GPSData() {
    mutex_init(&nmea_mutex_);
}

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

    // Print the raw data every second
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_print_time, now) >= 1000000) { // 1 second = 1,000,000 microseconds
        if (raw_data_index > 0) {
            raw_data_buffer[raw_data_index] = '\0'; // Null-terminate the string
            std::string raw_data_string(raw_data_buffer);
            uartPrint("Raw GPS data: " + raw_data_string);
            raw_data_index = 0; // Reset the index after printing
        }
        last_print_time = now; // Update the last print time
    }
}