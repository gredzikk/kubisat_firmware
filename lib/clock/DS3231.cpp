#include "DS3231.h"
#include "utils.h"
#include <stdio.h> 
#include <mutex>  
#include "event_manager.h"
#include "NMEA_data.h"

/**
 * @defgroup DS3231 RTC clock
 * @brief Functions for interfacing with the DS3231 RTC module
 * @{
 */

/**
 * @brief Constructor for the DS3231 class
 * 
 * @details Initializes the I2C interface and sets the device address for the DS3231 RTC module.
 *         The constructor is private to enforce the singleton pattern, ensuring that only one
 *        instance of the class can be created. The mutex for the class is also initialized.
 * @note The DS3231 device address is defined in the header file as DS3231_DEVICE_ADRESS.
 * @ingroup DS3231_RTC
 */
DS3231::DS3231() : i2c(MAIN_I2C_PORT), ds3231_addr(DS3231_DEVICE_ADRESS) {
    recursive_mutex_init(&clock_mutex_);
}


/**
 * @brief Gets the singleton instance of the DS3231 class
 * 
 * @return A reference to the singleton instance of the DS3231 class
 * @details This function provides access to the single instance of the DS3231
 *          class, ensuring that only one object manages the RTC module. The
 *          instance is created upon the first call to this function and remains
 *          available for the lifetime of the program.
 * @ingroup DS3231_RTC
 */
DS3231& DS3231::get_instance() {
    static DS3231 instance;
    return instance;
}

/**
 * @brief Sets the time on the DS3231 RTC module
 * 
 * @param[in] data Pointer to a ds3231_data_t structure containing the time to set
 * @return 0 on success, -1 on failure
 * @details Writes time and date data to the DS3231 module. The function performs
 *          input validation to ensure that the provided values are within valid
 *          ranges. The time values are converted from binary to BCD format
 *          before being written to the device registers.
 * @note The ds3231_data_t structure must contain valid values for seconds,
 *       minutes, hours, day, date, month, year, and century.
 * @ingroup DS3231_RTC
 */
int DS3231::set_time(ds3231_data_t *data) {
    uint8_t temp[7] = {0};

    if (clock_enable() != 0) {
        uart_print("Failed to enable clock oscillator", VerbosityLevel::ERROR);
        return -1;
    }

    if (data->seconds > 59)
        data->seconds = 59;
    if (data->minutes > 59)
        data->minutes = 59;
    if (data->hours > 23)
        data->hours = 23;
    if (data->day > 7)
        data->day = 7;
    else if (data->day < 1)
        data->day = 1;
    if (data->date > 31)
        data->date = 31;
    else if (data->date < 1)
        data->date = 1;
    if (data->month > 12)
        data->month = 12;
    else if (data->month < 1)
        data->month = 1;
    if (data->year > 99)
        data->year = 99;

    temp[0] = bin_to_bcd(data->seconds);
    temp[1] = bin_to_bcd(data->minutes);
    temp[2] = bin_to_bcd(data->hours);
    temp[2] &= ~(0x01 << 6); // Clear 12/24 hour bit
    temp[3] = bin_to_bcd(data->day);
    temp[4] = bin_to_bcd(data->date);
    temp[5] = bin_to_bcd(data->month);
    if (data->century)
        temp[5] |= (0x01 << 7);
    temp[6] = bin_to_bcd(data->year);

    std::string status = "BCD values to be written to DS3231: " + std::to_string(temp[0]) + " " + 
                        std::to_string(temp[1]) + " " + std::to_string(temp[2]) + " " + 
                        std::to_string(temp[3]) + " " + std::to_string(temp[4]) + " " + 
                        std::to_string(temp[5]) + " " + std::to_string(temp[6]);

    uart_print(status, VerbosityLevel::DEBUG);

    int result = i2c_write_reg(DS3231_SECONDS_REG, 7, temp);
    if (result != 0) {
        uart_print("i2c write failed", VerbosityLevel::ERROR);
        return -1;
    }

    return 0;
}


/**
 * @brief Reads the current time from the DS3231 RTC module
 * 
 * @param[out] data Pointer to a ds3231_data_t structure to store the read time
 * @return 0 on success, -1 on failure
 * @details Reads the time and date registers from the DS3231 and stores the
 *          decoded values in the provided data structure. The BCD values from
 *          the registers are converted to binary format. The function performs
 *          validation on the read values to ensure they are within valid ranges.
 * @note The function logs debug information including the raw BCD values read
 *       and the decoded time and date.
 * @ingroup DS3231_RTC
 */
int DS3231::get_time(ds3231_data_t *data) {
    std::string status;
    uint8_t raw_data[7];
    int result = i2c_read_reg(DS3231_SECONDS_REG, 7, raw_data);
    if (result != 0) {
        status = "Failed to read time from DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        return -1;
    }

    data->seconds = bcd_to_bin(raw_data[0] & 0x7F); // Masking for CH bit (clock halt)
    data->minutes = bcd_to_bin(raw_data[1] & 0x7F);
    data->hours = bcd_to_bin(raw_data[2] & 0x3F);   // Masking for 12/24 hour mode bit
    data->day = raw_data[3] & 0x07;                  // Day of week (1-7)
    data->date = bcd_to_bin(raw_data[4] & 0x3F);
    data->month = bcd_to_bin(raw_data[5] & 0x1F);   // Masking for century bit
    data->century = (raw_data[5] & 0x80) >> 7;
    data->year = bcd_to_bin(raw_data[6]);

    if (data->seconds > 59 || data->minutes > 59 || data->hours > 23 ||
        data->day < 1 || data->day > 7 || data->date < 1 || data->date > 31 ||
        data->month < 1 || data->month > 12 || data->year > 99) {
        uart_print("Invalid data read from DS3231", VerbosityLevel::ERROR);
        return -1;
    }

    return 0;
}


/**
 * @brief Reads the temperature from the DS3231's internal temperature sensor
 * 
 * @param[out] resolution Pointer to a float to store the temperature value
 * @return 0 on success, -1 on failure
 * @details The DS3231 includes an internal temperature sensor with 0.25°C resolution.
 *          This function reads the sensor value and calculates the temperature in
 *          degrees Celsius. The temperature sensor is primarily used for the
 *          oscillator's temperature compensation, but can be used for general
 *          temperature monitoring as well.
 * @ingroup DS3231_RTC
 */
int DS3231::read_temperature(float *resolution) {
    std::string status;
    uint8_t temp[2];
    int result = i2c_read_reg(DS3231_TEMPERATURE_MSB_REG, 2, temp);
    if (result != 0) {
        status = "Failed to read temperature from DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        return -1;
    }

    int8_t temperature_msb = (int8_t)temp[0]; 
    uint8_t temperature_lsb = temp[1] >> 6;    // Only the 2 MSB are valid

    *resolution = temperature_msb + (temperature_lsb * 0.25f); // 0.25 degree resolution

    return 0;
}


/**
 * @brief Sets the DS3231 clock using a Unix timestamp
 * 
 * @param[in] unix_time The time in seconds since the Unix epoch (1970-01-01 00:00:00 UTC)
 * @return 0 on success, -1 on failure
 * @details Converts the provided Unix timestamp to a calendar date and time
 *          and sets the DS3231 RTC accordingly. This function properly handles
 *          the conversion between the tm structure (used by C standard library)
 *          and the internal ds3231_data_t format.
 * @ingroup DS3231_RTC
 */
int DS3231::set_unix_time(time_t unix_time) {
    struct tm *timeinfo = gmtime(&unix_time);
    if (timeinfo == NULL) {
        uart_print("Error: gmtime() failed", VerbosityLevel::ERROR);
        return -1;
    }

    ds3231_data_t data;
    data.seconds = timeinfo->tm_sec;
    data.minutes = timeinfo->tm_min;
    data.hours = timeinfo->tm_hour;
    data.day = timeinfo->tm_wday == 0 ? 7 : timeinfo->tm_wday; // Sunday is 0 in tm struct, but 1 in DS3231
    data.date = timeinfo->tm_mday;
    data.month = timeinfo->tm_mon + 1; // Month is 0-11 in tm struct, but 1-12 in DS3231
    data.year = timeinfo->tm_year - 100; // Year is since 1900, we want the last two digits
    data.century = timeinfo->tm_year >= 2000;

    return set_time(&data);
}


/**
 * @brief Gets the current time from DS3231 as a Unix timestamp
 * 
 * @return Unix timestamp (seconds since 1970-01-01 00:00:00 UTC), or -1 on error
 * @details Reads the current time from the DS3231 RTC and converts it to a Unix
 *          timestamp. This function properly handles the conversion between the 
 *          internal ds3231_data_t format and the tm structure used by the C
 *          standard library.
 * @ingroup DS3231_RTC
 */
time_t DS3231::get_unix_time() {
    ds3231_data_t data;
    if (get_time(&data)) {
        return -1; 
    }

    struct tm timeinfo;
    timeinfo.tm_sec = data.seconds;
    timeinfo.tm_min = data.minutes;
    timeinfo.tm_hour = data.hours;
    timeinfo.tm_mday = data.date;
    timeinfo.tm_mon = data.month - 1; // Month is 0-11 in tm struct, but 1-12 in DS3231
    timeinfo.tm_year = data.year + 100; // Year is since 1900

    // mktime assumes that tm_wday and tm_yday are uninitialized
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_isdst = 0; // Set to 0 to use UTC

    time_t timestamp = mktime(&timeinfo);
    if (timestamp == (time_t)(-1)) {
        uart_print("Error: mktime() failed", VerbosityLevel::ERROR);
        return -1;
    }

    return timestamp;
}


/**
 * @brief Enables the DS3231's oscillator
 * 
 * @return 0 on success, -1 on failure
 * @details Reads the control register and clears the EOSC (Enable Oscillator) bit
 *          to ensure the oscillator is running. This is necessary for the RTC to 
 *          keep time when not on external power.
 * @ingroup DS3231_RTC
 */
int DS3231::clock_enable() {
    std::string status;
    uint8_t control_reg = 0;
    int result = i2c_read_reg(DS3231_CONTROL_REG, 1, &control_reg);
    if (result != 0) {
        status = "Failed to read control register";
        uart_print(status, VerbosityLevel::ERROR);
        return -1;
    }

    // Clear the EOSC bit to enable the oscillator
    control_reg &= ~(1 << 7);

    result = i2c_write_reg(DS3231_CONTROL_REG, 1, &control_reg);
    if (result != 0) {
        status = "Failed to write control register";
        uart_print(status, VerbosityLevel::ERROR);
        return -1;
    }

    return 0;
}


/**
 * @brief Gets the currently configured timezone offset
 * 
 * @return The timezone offset in minutes
 * @details Returns the current timezone offset in minutes relative to UTC.
 *          Positive values represent timezones ahead of UTC (east),
 *          negative values represent timezones behind UTC (west).
 * @ingroup DS3231_RTC
 */
int16_t DS3231::get_timezone_offset() const {
    return timezone_offset_minutes_;
}


/**
 * @brief Sets the timezone offset
 * 
 * @param[in] offset_minutes The timezone offset in minutes
 * @details Sets the timezone offset in minutes relative to UTC. This value
 *          is used when converting between UTC and local time. The function
 *          validates that the offset is within a valid range (-720 to +720 minutes,
 *          which corresponds to -12 to +12 hours).
 * @note This setting is stored in memory and does not persist across reboots.
 * @ingroup DS3231_RTC
 */
void DS3231::set_timezone_offset(int16_t offset_minutes) {
    // Validate range: -12 hours to +12 hours (-720 to +720 minutes)
    if (offset_minutes >= -720 && offset_minutes <= 720) {
        timezone_offset_minutes_ = offset_minutes;
    } else {
        uart_print("Error: Invalid timezone offset", VerbosityLevel::ERROR);
    }
}


/**
 * @brief Gets the currently configured clock synchronization interval
 * 
 * @return The sync interval in minutes
 * @details Returns the current interval between clock synchronization attempts.
 *          This is the time after which is_sync_needed() will return true.
 * @ingroup DS3231_RTC
 */
uint32_t DS3231::get_clock_sync_interval() const {
    return sync_interval_minutes_;
}


/**
 * @brief Sets the clock synchronization interval
 * 
 * @param[in] interval_minutes The desired sync interval in minutes
 * @details Sets how frequently the clock should be synchronized with an external
 *          time source (such as GPS). The function validates that the interval is
 *          within a valid range (1 minute to 43200 minutes, which is 30 days).
 * @note This setting is stored in memory and does not persist across reboots.
 * @ingroup DS3231_RTC
 */
void DS3231::set_clock_sync_interval(uint32_t interval_minutes) {
    if (interval_minutes >= 1 && interval_minutes <= 43200) {
        sync_interval_minutes_ = interval_minutes;
    } else {
        uart_print("Error: Invalid sync interval", VerbosityLevel::ERROR);
    }
}


/**
 * @brief Gets the timestamp of the last successful clock synchronization
 * 
 * @return Unix timestamp of the last sync, or 0 if never synced
 * @details Returns the Unix timestamp of when the clock was last successfully
 *          synchronized with an external time source. A value of 0 indicates
 *          that the clock has never been synchronized.
 * @ingroup DS3231_RTC
 */
time_t DS3231::get_last_sync_time() const {
    return last_sync_time_;
}


/**
 * @brief Updates the last sync timestamp to the current time
 * 
 * @details Records the current time as the last successful synchronization time.
 *          This should be called after successfully setting the time from an
 *          external source (such as GPS). The function logs the update with an
 *          informational message.
 * @ingroup DS3231_RTC
 */
void DS3231::update_last_sync_time() {
    last_sync_time_ = get_unix_time();
    uart_print("Clock sync time updated: " + std::to_string(last_sync_time_), VerbosityLevel::INFO);
}


/**
 * @brief Gets the current local time by applying the timezone offset to UTC time
 * 
 * @return Local time as Unix timestamp, or -1 on error
 * @details Retrieves the current UTC time from the RTC and applies the configured
 *          timezone offset (in minutes) to calculate the local time.
 * @ingroup DS3231_RTC
 */
time_t DS3231::get_local_time() {
    time_t utc_time = get_unix_time();
    if (utc_time == -1) {
        return -1;
    }
    
    return utc_time + (timezone_offset_minutes_ * 60);
}


/**
 * @brief Determines if the clock needs synchronization based on the configured interval
 * 
 * @return true if synchronization is needed, false otherwise
 * @details This method checks if the clock has ever been synchronized (last_sync_time_ is 0)
 *          or if the time elapsed since the last synchronization exceeds the configured
 *          sync_interval_minutes_. If the current time cannot be determined, it assumes
 *          synchronization is needed.
 * @ingroup DS3231_RTC
 */
bool DS3231::is_sync_needed() {
    if (last_sync_time_ == 0) {
        return true;
    }
    
    time_t current_time = get_unix_time();
    if (current_time == -1) {
        return true;
    }
    
    time_t time_since_last_sync = current_time - last_sync_time_;
    uint32_t minutes_since_last_sync = time_since_last_sync / 60;
    
    return minutes_since_last_sync >= sync_interval_minutes_;
}


/**
 * @brief Synchronizes the RTC with time from GPS
 * 
 * @param[in] nmea_data Reference to NMEA data containing time information
 * @return true if synchronization succeeded, false if it failed
 * @details This method attempts to extract valid time data from the provided NMEA data
 *          and use it to update the RTC. It performs validity checks on the GPS data
 *          before attempting synchronization. If successful, it updates the last sync
 *          time and emits a SYNCED event. If unsuccessful, it emits a SYNC_FAILED event.
 * 
 * @note This function emits events to the EventEmitter system that can be monitored
 *       by other components of the system.
 * @ingroup DS3231_RTC
 */
bool DS3231::sync_clock_with_gps() {
    auto& nmea_data = NMEAData::get_instance(); 
    
    if (!nmea_data.has_valid_time()) {
        uart_print("GPS time data not available for sync", VerbosityLevel::WARNING);
        EventEmitter::emit(EventGroup::CLOCK, ClockEvent::GPS_SYNC_DATA_NOT_READY);
        return false;
    }
    
    time_t gps_time = nmea_data.get_unix_time();
    if (gps_time <= 0) {
        uart_print("Invalid GPS time for sync", VerbosityLevel::ERROR);
        EventEmitter::emit(EventGroup::CLOCK, ClockEvent::GPS_SYNC_DATA_NOT_READY);
        return false;
    }
    
    if (set_unix_time(gps_time) != 0) {
        uart_print("Failed to set system time from GPS", VerbosityLevel::ERROR);
        EventEmitter::emit(EventGroup::CLOCK, ClockEvent::GPS_SYNC_DATA_NOT_READY);
        return false;
    }
    
    update_last_sync_time();
    
    EventEmitter::emit(EventGroup::CLOCK, ClockEvent::GPS_SYNC);
    uart_print("Clock synced with GPS time: " + std::to_string(gps_time), VerbosityLevel::INFO);
    
    return true;
}

// ==================== private methods

/**
 * @brief Reads data from a specific register on the DS3231
 * 
 * @param[in] reg_addr Register address to read from
 * @param[in] length Number of bytes to read
 * @param[out] data Buffer to store read data
 * @return 0 on success, -1 on failure
 * @details This method performs a thread-safe I²C read operation from the DS3231.
 *          It first writes the register address to the device, then reads the
 *          requested number of bytes. All access is protected by a mutex to prevent
 *          concurrent I²C operations that could corrupt data.
 * 
 * @note This is a low-level method used internally by the class.
 * @ingroup DS3231_RTC
 */
int DS3231::i2c_read_reg(uint8_t reg_addr, size_t length, uint8_t *data) {
    if (!length)
        return -1;

    std::string status = "Reading register " + std::to_string(reg_addr) + " from DS3231";
    uart_print(status, VerbosityLevel::DEBUG);
    recursive_mutex_enter_blocking(&clock_mutex_);
    uint8_t reg = reg_addr;
    int write_result = i2c_write_blocking(i2c, ds3231_addr, &reg, 1, true);
    if (write_result == PICO_ERROR_GENERIC) {
        status = "Failed to write register address to DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    int read_result = i2c_read_blocking(i2c, ds3231_addr, data, length, false);
    if (read_result == PICO_ERROR_GENERIC) {
        status = "Failed to read register data from DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    recursive_mutex_exit(&clock_mutex_);

    return 0;
}

/**
 * @brief Writes data to a specific register on the DS3231
 * 
 * @param[in] reg_addr Register address to write to
 * @param[in] length Number of bytes to write
 * @param[in] data Buffer containing data to write
 * @return 0 on success, -1 on failure
 * @details This method performs a thread-safe I²C write operation to the DS3231.
 *          It combines the register address and data into a single buffer and
 *          sends it to the device. All access is protected by a mutex to prevent
 *          concurrent I²C operations that could corrupt data.
 * 
 * @note This is a low-level method used internally by the class.
 * @ingroup DS3231_RTC
 */
int DS3231::i2c_write_reg(uint8_t reg_addr, size_t length, uint8_t *data) {
    if (!length)
        return -1;

    recursive_mutex_enter_blocking(&clock_mutex_);
    std::vector<uint8_t> message(length + 1);
    message[0] = reg_addr;
    for (size_t i = 0; i < length; i++) {
        message[i + 1] = data[i];
    }
    int write_result = i2c_write_blocking(i2c, ds3231_addr, message.data(), (length + 1), false);
    if (write_result == PICO_ERROR_GENERIC) {
        uart_print("Error: i2c_write_blocking failed in i2c_write_reg", VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    recursive_mutex_exit(&clock_mutex_);

    return 0;
}

/**
 * @brief Converts binary value to Binary-Coded Decimal (BCD) format
 * 
 * @param[in] data Binary value to convert (0-99)
 * @return BCD representation of the input value
 * @details The DS3231 stores time values in BCD format where each nibble represents
 *          a decimal digit. This function converts a standard binary value to its
 *          BCD equivalent (e.g., 42 becomes 0x42).
 * @ingroup DS3231_RTC
 */
uint8_t DS3231::bin_to_bcd(const uint8_t data) {
    uint8_t ones_digit = (uint8_t)(data % 10);
    uint8_t tens_digit = (uint8_t)(data - ones_digit) / 10;
    return ((tens_digit << 4) + ones_digit);
}


/**
 * @brief Converts Binary-Coded Decimal (BCD) to binary value
 * 
 * @param[in] bcd BCD value to convert
 * @return Binary representation of the input BCD value
 * @details The DS3231 stores time values in BCD format where each nibble represents
 *          a decimal digit. This function converts a BCD value to its standard binary
 *          equivalent (e.g., 0x42 becomes 42).
 * @ingroup DS3231_RTC
 */
uint8_t DS3231::bcd_to_bin(const uint8_t bcd) {
    uint8_t ones_digit = (uint8_t)(bcd & 0x0F);
    uint8_t tens_digit = (uint8_t)(bcd >> 4);
    return (tens_digit * 10 + ones_digit);
}
/** @} */ // End of DS3231_RTC group