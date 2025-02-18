#include "DS3231.h"
#include <iomanip>
#include <sstream>
const std::array<std::string, 7> DS3231::WEEKDAYS = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

DS3231::DS3231(i2c_inst_t *i2c_port, uint8_t address) : i2c(i2c_port), address(address) {
}

bool DS3231::setTime(uint8_t sec, uint8_t min, uint8_t hour,
                    uint8_t weekday, uint8_t day, uint8_t month, uint16_t year) {
    uint8_t buffer[8];
    buffer[0] = 0;
    buffer[1] = bin2bcd(sec);
    buffer[2] = bin2bcd(min);
    buffer[3] = bin2bcd(hour);
    buffer[4] = bin2bcd(weekday);
    buffer[5] = bin2bcd(day);
    buffer[6] = bin2bcd(month);
    buffer[7] = bin2bcd(year - 2000);
    // Write time to RTC
    uint8_t reg = RTC_REGISTER;
    i2c_write_blocking(i2c, address, &reg, 1, true);
    return i2c_write_blocking(i2c, address, buffer, sizeof(buffer), false) == sizeof(buffer);
}

bool DS3231::getTime(uint8_t& sec, uint8_t& min, uint8_t& hour,
                    std::string& weekday, uint8_t& day, uint8_t& month, uint16_t& year) {
    uint8_t buffer[7];
    uint8_t reg = RTC_REGISTER;
    
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) {
        return false;
    }
    
    if (i2c_read_blocking(i2c, address, buffer, sizeof(buffer), false) != sizeof(buffer)) {
        return false;
    }

    sec = bcd2bin(buffer[0]);
    min = bcd2bin(buffer[1]);
    hour = bcd2bin(buffer[2]);
    weekday = WEEKDAYS[bcd2bin(buffer[3])];
    day = bcd2bin(buffer[4]);
    month = bcd2bin(buffer[5]);
    year = bcd2bin(buffer[6]) + 2000;
    
    return true;
}

std::string DS3231::getTimeString() {
    uint8_t sec, min, hour, day, month;
    uint16_t year;
    std::string weekday;
    
    if (!getTime(sec, min, hour, weekday, day, month, year)) {
        return "Error reading RTC";
    }
    
    std::stringstream ss;
    ss << preZero(hour) << ":" << preZero(min) << ":" << preZero(sec)
       << "      " << weekday << " " 
       << static_cast<int>(day) << "." 
       << static_cast<int>(month) << "." 
       << year;
    
    return ss.str();
}

uint8_t DS3231::bcd2bin(uint8_t val) {
    return ((val/16) * 10) + (val % 16);
}

uint8_t DS3231::bin2bcd(uint8_t val) {
    return ((val/10) * 16) + (val % 10);
}

std::string DS3231::preZero(uint8_t val) {
    if (val < 10) {
        return "0" + std::to_string(val);
    }
    return std::to_string(val);
}

bool DS3231::setTimeUnix(uint32_t unixTime) {
    DateTime dt = unixToDateTime(unixTime);
    uint8_t weekdayIndex = 0;
    // Calculate weekday (0 = Sunday, 1 = Monday, etc.)
    time_t t = unixTime;
    struct tm* tmp = gmtime(&t);
    weekdayIndex = tmp->tm_wday;
    
    return setTime(dt.second, dt.minute, dt.hour, 
                  weekdayIndex, dt.day, dt.month, dt.year);
}

uint32_t DS3231::getTimeUnix() {
    DateTime dt = getDateTime();
    return dateTimeToUnix(dt);
}

DateTime DS3231::getDateTime() {
    DateTime dt;
    uint8_t buffer[7];
    uint8_t reg = RTC_REGISTER;
    
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) {
        return DateTime{0};
    }
    
    if (i2c_read_blocking(i2c, address, buffer, sizeof(buffer), false) != sizeof(buffer)) {
        return DateTime{0};
    }

    dt.second = bcd2bin(buffer[0]);
    dt.minute = bcd2bin(buffer[1]);
    dt.hour = bcd2bin(buffer[2]);
    dt.weekday = WEEKDAYS[bcd2bin(buffer[3])];
    dt.day = bcd2bin(buffer[4]);
    dt.month = bcd2bin(buffer[5]);
    dt.year = bcd2bin(buffer[6]) + 2000;
    
    return dt;
}

uint64_t DS3231::getTimeInteger() {
    DateTime dt = getDateTime();
    return static_cast<uint64_t>(dt.year) * 10000000000ULL +
           static_cast<uint64_t>(dt.month) * 100000000ULL +
           static_cast<uint64_t>(dt.day) * 1000000ULL +
           static_cast<uint64_t>(dt.hour) * 10000ULL +
           static_cast<uint64_t>(dt.minute) * 100ULL +
           static_cast<uint64_t>(dt.second);
}

uint32_t DS3231::dateTimeToUnix(const DateTime& dt) {
    struct tm timeinfo = {};
    timeinfo.tm_year = dt.year - 1900;
    timeinfo.tm_mon = dt.month - 1;
    timeinfo.tm_mday = dt.day;
    timeinfo.tm_hour = dt.hour;
    timeinfo.tm_min = dt.minute;
    timeinfo.tm_sec = dt.second;
    return mktime(&timeinfo);
}

DateTime DS3231::unixToDateTime(uint32_t unixTime) {
    DateTime dt;
    time_t t = unixTime;
    struct tm* tmp = gmtime(&t);
    
    dt.year = tmp->tm_year + 1900;
    dt.month = tmp->tm_mon + 1;
    dt.day = tmp->tm_mday;
    dt.hour = tmp->tm_hour;
    dt.minute = tmp->tm_min;
    dt.second = tmp->tm_sec;
    dt.weekday = WEEKDAYS[tmp->tm_wday];
    
    return dt;
}

uint32_t DS3231::getTimeUnixLocal() {
    uint32_t utcTime = getTimeUnix();
    return utcTime + (timezoneOffset * 60); // Convert minutes to seconds
}

DateTime DS3231::getDateTimeLocal() {
    DateTime utc = getDateTime();
    return applyTimezone(utc, timezoneOffset);
}

DateTime DS3231::applyTimezone(const DateTime& utc, int16_t offsetMinutes) {
    time_t t = dateTimeToUnix(utc) + (offsetMinutes * 60);
    return unixToDateTime(t);
}

bool DS3231::needsSync() {
    uint32_t now = getTimeUnix();
    
    // Check if sync interval has passed
    if (now - lastSyncTime >= syncInterval) {
        return true;
    }
    
    // Calculate drift since last sync
    float driftSeconds = (now - lastSyncTime) * (clockDrift / 1000000.0f);
    
    // If drift is more than 1 second, sync is needed
    return std::abs(driftSeconds) >= 1.0f;
}