#ifndef DS3231_H
#define DS3231_H

#include <string>
#include <array>
#include <chrono>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

using Weekday = enum class Weekday {
    SUN, MON, TUE, WED, THU, FRI, SAT
};

using DateTime = struct DateTime {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    Weekday weekday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

class DS3231 {
public:
    DS3231(i2c_inst_t *i2c, uint8_t address = 0x68);

    bool setTime(const DateTime& dateTime) const;
    bool getTime(DateTime& dateTime) const;
    std::string getTimeString() const;
    static std::string weekdayToString(Weekday weekday);

private:
    i2c_inst_t* i2c;
    uint8_t address;
    static constexpr uint8_t RTC_REGISTER = 0x00;

    uint8_t bcd2bin(uint8_t val) const;
    uint8_t bin2bcd(uint8_t val) const;
    bool i2cWrite(uint8_t reg, const std::array<uint8_t, 8>& data) const;
    bool i2cRead(uint8_t reg, std::array<uint8_t, 7>& data) const;
    bool validateDateTime(const DateTime& dateTime) const;
    bool isLeapYear(uint16_t year) const;
    Weekday dayOfWeek(uint16_t year, uint8_t month, uint8_t day) const;
};

#endif