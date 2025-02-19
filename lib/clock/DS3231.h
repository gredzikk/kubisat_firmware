#ifndef DS3231_H
#define DS3231_H

#include <string>
#include <array>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <time.h>
#include "pico/mutex.h"

#define DS3231_DEVICE_ADRESS            0x68

#define DS3231_SECONDS_REG              0x00
#define DS3231_MINUTES_REG              0x01
#define DS3231_HOURS_REG                0x02
#define DS3231_DAY_REG                  0x03
#define DS3231_DATE_REG                 0x04
#define DS3231_MONTH_REG                0x05
#define DS3231_YEAR_REG                 0x06

#define DS3231_CONTROL_REG              0x0E
#define DS3231_CONTROL_STATUS_REG       0x0F

#define DS3231_TEMPERATURE_MSB_REG      0x11
#define DS3231_TEMPERATURE_LSB_REG      0x12

enum days_of_week {
    MONDAY  = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
};

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    bool century;
} ds3231_data_t;

class DS3231 {
public:
    DS3231(i2c_inst_t *i2c_instance);

    int set_time(ds3231_data_t *data);
    int get_time(ds3231_data_t *data);
    int read_temperature(float *resolution);

    int set_unix_time(time_t unix_time);
    time_t get_unix_time();
    int clock_enable();

private:
    i2c_inst_t *i2c;
    uint8_t ds3231_addr;

    int i2c_read_reg(uint8_t reg_addr, size_t length, uint8_t *data);
    int i2c_write_reg(uint8_t reg_addr, size_t length, uint8_t *data);

    uint8_t bin_to_bcd(const uint8_t data);
    uint8_t bcd_to_bin(const uint8_t bcd);

    recursive_mutex_t clock_mutex_; // Mutex for I2C access
};

#endif