#ifndef GPS_COLLECTOR_H
#define GPS_COLLECTOR_H

#include <string>
#include "hardware/uart.h"
#include "lib/location/NMEA/nmea_data.h" // Include the new header
#include "pin_config.h"

// Function to collect GPS data from the UART
void collect_gps_data();

#endif