#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "event_manager.h"
#include "lib/powerman/PowerManager.h" // Corrected path
#include <pico/bootrom.h>

#include "ISensor.h"
#include "lib/sensors/BH1750/BH1750_WRAPPER.h" // Corrected path
#include "lib/sensors/BME280/BME280_WRAPPER.h" // Corrected path
#include "lib/sensors/HMC5883L/HMC5883L_WRAPPER.h" // Corrected path
#include "lib/sensors/MPU6050/MPU6050_WRAPPER.h" // Corrected path
#include "lib/clock/DS3231.h" // Corrected path
#include <iostream>
#include <iomanip>
#include <queue>
#include <chrono>
#include "protocol.h"
#include <atomic>
#include <iostream>
#include <map>
#include "pin_config.h"
#include "utils.h"
#include "communication.h"
#include "build_number.h"
#include "lib/location/gps_collector.h"
#include "lib/storage/storage.h" // Corrected path
#include "lib/storage/pico-vfs/include/filesystem/vfs.h" // Corrected path
#include "telemetry_manager.h"
#include "system_state_manager.h"

#endif