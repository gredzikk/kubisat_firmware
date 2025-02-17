#ifndef GPS_COLLECTOR_H
#define GPS_COLLECTOR_H

#include <string>
#include "hardware/uart.h"
#include "lib/GPS/NMEA/nmea_data.h" // Include the new header
#include "lib/GPS/NMEA/NMEA_parser.h"
#include "pin_config.h"

// Function to collect GPS data from the UART
void collectGPSData();

#endif