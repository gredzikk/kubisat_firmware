#include "DS3231.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <array>

DS3231::DS3231(i2c_inst_t *i2c_port, uint8_t address) : i2c(i2c_port), address(address) {
}

bool DS3231::setTime(const DateTime& dateTime) const {
    if (!validateDateTime(dateTime)) {
        return false;
    }

    std::array<uint8_t, 8> buffer;
    buffer[0] = 0;
    buffer[1] = bin2bcd(dateTime.sec);
    buffer[2] = bin2bcd(dateTime.min);
    buffer[3] = bin2bcd(dateTime.hour);
    buffer[4] = bin2bcd(static_cast<uint8_t>(dateTime.weekday));
    buffer[5] = bin2bcd(dateTime.day);
    buffer[6] = bin2bcd(dateTime.month);
    buffer[7] = bin2bcd(static_cast<uint8_t>(dateTime.year - 2000));

    return i2cWrite(RTC_REGISTER, buffer);
}

bool DS3231::getTime(DateTime& dateTime) const {
    std::array<uint8_t, 7> buffer;
    if (!i2cRead(RTC_REGISTER, buffer)) {
        return false;
    }

    dateTime.sec = bcd2bin(buffer[0]);
    dateTime.min = bcd2bin(buffer[1]);
    dateTime.hour = bcd2bin(buffer[2]);
    dateTime.weekday = static_cast<Weekday>(bcd2bin(buffer[3]));
    dateTime.day = bcd2bin(buffer[4]);
    dateTime.month = bcd2bin(buffer[5]);
    dateTime.year = bcd2bin(buffer[6]) + 2000;

    return true;
}

std::string DS3231::getTimeString() const {
    DateTime dateTime;

    if (!getTime(dateTime)) {
        return "Error reading RTC";
    }

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(dateTime.hour) << ":"
       << std::setw(2) << std::setfill('0') << static_cast<int>(dateTime.min) << ":"
       << std::setw(2) << std::setfill('0') << static_cast<int>(dateTime.sec)
       << " " << static_cast<int>(dateTime.weekday) << " " 
       << static_cast<int>(dateTime.day) << "." 
       << static_cast<int>(dateTime.month) << "." 
       << dateTime.year;

    return ss.str();
}

uint8_t DS3231::bcd2bin(uint8_t val) const {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t DS3231::bin2bcd(uint8_t val) const {
    return ((val / 10) * 16) + (val % 10);
}

bool DS3231::i2cWrite(uint8_t reg, const std::array<uint8_t, 8>& data) const {
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) {
        return false;
    }
    return i2c_write_blocking(i2c, address, data.data(), data.size(), false) == data.size();
}

bool DS3231::i2cRead(uint8_t reg, std::array<uint8_t, 7>& data) const {
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) {
        return false;
    }
    return i2c_read_blocking(i2c, address, data.data(), data.size(), false) == data.size();
}

bool DS3231::validateDateTime(const DateTime& dateTime) const {
    if (dateTime.sec > 59 || dateTime.min > 59 || dateTime.hour > 23) {
        return false;
    }

    if (dateTime.month < 1 || dateTime.month > 12 || dateTime.day < 1) {
        return false;
    }

    static const std::array<uint8_t, 12> daysInMonth = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    uint8_t maxDays = daysInMonth[dateTime.month - 1];

    if (dateTime.month == 2 && isLeapYear(dateTime.year)) {
        maxDays = 29;
    }

    if (dateTime.day > maxDays) {
        return false;
    }

    if (dayOfWeek(dateTime.year, dateTime.month, dateTime.day) != dateTime.weekday) {
        return false;
    }

    return true;
}

bool DS3231::isLeapYear(uint16_t year) const {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

Weekday DS3231::dayOfWeek(uint16_t year, uint8_t month, uint8_t day) const {
    static const std::array<uint8_t, 12> monthOffsetsLeapYear = { 0, 3, 4, 0, 2, 5, 0, 3, 6, 1, 4, 6 };
    static const std::array<uint8_t, 12> monthOffsetsNonLeapYear = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

    const auto& monthOffsets = isLeapYear(year) ? monthOffsetsLeapYear : monthOffsetsNonLeapYear;
    uint8_t monthOffset = monthOffsets[month - 1];

    year -= 1;
    uint8_t weekday = (day + monthOffset + 5 * (year % 4) + 4 * (year % 100) + 6 * (year % 400)) % 7;

    return static_cast<Weekday>(weekday);
}

std::string DS3231::weekdayToString(Weekday weekday) {
    switch (weekday) {
        case Weekday::SUN: return "Sun";
        case Weekday::MON: return "Mon";
        case Weekday::TUE: return "Tue";
        case Weekday::WED: return "Wed";
        case Weekday::THU: return "Thu";
        case Weekday::FRI: return "Fri";
        case Weekday::SAT: return "Sat";
        default: return "Unknown";
    }
}