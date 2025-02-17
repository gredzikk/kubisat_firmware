#include "lib/GPS/NMEA/nmea_data.h"

NMEAData nmea_data; // Define the global instance

NMEAData::NMEAData() {
    mutex_init(&nmea_mutex_);
    mutex_init(&parsed_data_mutex_);
}