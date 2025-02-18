#ifndef DS3231_H
#define DS3231_H

#include <string>
#include <array>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    std::string weekday;
};

class DS3231 {
public:
    // Weekday names
    static const std::array<std::string, 7> WEEKDAYS;
    
    // Constructor
    DS3231(i2c_inst_t *i2c, uint8_t address = 0x68);

    // Set time using binary-coded decimal format
    bool setTime(uint8_t sec, uint8_t min, uint8_t hour, 
                 uint8_t weekday, uint8_t day, uint8_t month, uint16_t year);
    
    // Get time in two formats
    bool getTime(uint8_t& sec, uint8_t& min, uint8_t& hour,
                std::string& weekday, uint8_t& day, uint8_t& month, uint16_t& year);
    std::string getTimeString();
    bool setTimeUnix(uint32_t unixTime);
    uint32_t getTimeUnix();
    DateTime getDateTime();
    uint64_t getTimeInteger(); // Returns YYYYMMDDHHMMSS

    void setTimezoneOffset(int16_t offsetMinutes) { timezoneOffset = offsetMinutes; }
    int16_t getTimezoneOffset() const { return timezoneOffset; }
    
    uint32_t getTimeUnixLocal();
    DateTime getDateTimeLocal();
    
    void setClockSyncInterval(uint32_t intervalSeconds) { syncInterval = intervalSeconds; }
    uint32_t getClockSyncInterval() const { return syncInterval; }
    
    void setLastSyncTime(uint32_t unixTime) { lastSyncTime = unixTime; }
    uint32_t getLastSyncTime() const { return lastSyncTime; }


private:
    i2c_inst_t* i2c;
    uint8_t address;
    static constexpr uint8_t RTC_REGISTER = 0x00;

    int16_t timezoneOffset = 0;      // Offset in minutes from UTC
    uint32_t syncInterval = 86400;    // Default sync interval: 24 hours
    uint32_t lastSyncTime = 0;        // Last successful GPS sync time
    
    // Utility functions
    uint8_t bcd2bin(uint8_t val);
    uint8_t bin2bcd(uint8_t val);
    std::string preZero(uint8_t val);
    static uint32_t dateTimeToUnix(const DateTime& dt);
    static DateTime unixToDateTime(uint32_t unixTime);
    DateTime applyTimezone(const DateTime& utc, int16_t offsetMinutes);

};

#endif