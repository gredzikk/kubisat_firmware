#include "DS3231.h"
#include "utils.h"

DS3231::DS3231(i2c_inst_t *i2c_instance) : i2c(i2c_instance), ds3231_addr(DS3231_DEVICE_ADRESS) {}

int DS3231::set_time(ds3231_data_t *data) {
    uint8_t temp[7] = {0};
    if (i2c_read_reg(DS3231_SECONDS_REG, 7, temp))
        return -1;

    uart_print("Time read, converting to BCD and writing");

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
    temp[2] &= ~(0x01 << 6);
    temp[3] = bin_to_bcd(data->day);
    temp[4] = bin_to_bcd(data->date);
    temp[5] = bin_to_bcd(data->month);

    if (data->century)
        temp[5] |= (0x01 << 7);

    temp[6] = bin_to_bcd(data->year);

    uart_print("Writing time to DS3231");
    std::string timeStr = "Time: " + std::to_string(data->hours) + ":" + std::to_string(data->minutes) + ":" + std::to_string(data->seconds);
    uart_print(timeStr);
    std::string dateStr = "Date: " + std::to_string(data->date) + "/" + std::to_string(data->month) + "/" + std::to_string(data->year);
    uart_print(dateStr);
    if (i2c_write_reg(DS3231_SECONDS_REG, 7, temp))
        return -1;
    return 0;
}

int DS3231::get_time(ds3231_data_t *data) {
    uint8_t raw_data[7];
    if (i2c_read_reg(DS3231_SECONDS_REG, 7, raw_data))
        return -1;

    data->seconds = bcd_to_bin(raw_data[0] & 0x7F); // Masking for CH bit (clock halt)
    data->minutes = bcd_to_bin(raw_data[1] & 0x7F);
    data->hours = bcd_to_bin(raw_data[2] & 0x3F);   // Masking for 12/24 hour mode bit
    data->day = raw_data[3] & 0x07;                  // Day of week (1-7)
    data->date = bcd_to_bin(raw_data[4] & 0x3F);
    data->month = bcd_to_bin(raw_data[5] & 0x1F);   // Masking for century bit
    data->century = (raw_data[5] & 0x80) >> 7;
    data->year = bcd_to_bin(raw_data[6]);

    uart_print("Reading time from DS3231");
    std::string timeStr = "Time: " + std::to_string(data->hours) + ":" + std::to_string(data->minutes) + ":" + std::to_string(data->seconds);
    uart_print(timeStr);
    std::string dateStr = "Date: " + std::to_string(data->date) + "/" + std::to_string(data->month) + "/" + std::to_string(data->year);
    uart_print(dateStr);

    return 0;
}

int DS3231::read_temperature(float *resolution) {
    uint8_t temp[2] = {0, 0};
    if (i2c_read_reg(DS3231_TEMPERATURE_MSB_REG, 2, temp))
        return -1;

    *resolution = temp[0] + (float)(1 / (temp[1] >> 6));
    return 0;
}

int DS3231::set_unix_time(time_t unix_time) {
    struct tm *timeinfo = gmtime(&unix_time);
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

time_t DS3231::get_unix_time() {
    ds3231_data_t data;
    if (get_time(&data)) {
        return -1; // Indicate error
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

    return mktime(&timeinfo);
}

/**
 * @brief               Library function to read a specific I2C register adress.
 *
 * @param[in] reg_addr  Register adress to be read.
 * @param[in] length    length of the data in bytes to be read.
 * @param[out] data     Buffer to store the read data.
 * @return              0 if succesful, -1 if i2c failure.
 */
int DS3231::i2c_read_reg(uint8_t reg_addr, size_t length, uint8_t *data) {
    if (!length)
        return -1;
    uint8_t reg = reg_addr;
    if (i2c_write_blocking(i2c, ds3231_addr, &reg, 1, true) == PICO_ERROR_GENERIC) {
        return -1;
    }
    if (i2c_read_blocking(i2c, ds3231_addr, data, length, false) == PICO_ERROR_GENERIC) {
        return -1;
    }
    return 0;
}

/**
 * @brief               Library function to write to a specific I2C register adress.
 *
 * @param[in] reg_addr  Register adress to be written.
 * @param[in] length    Length of the data to be written in bytes.
 * @param[in] data      Pointer to the data buffer.
 * @return              0 if succesful, -1 if i2c failure.
 */
int DS3231::i2c_write_reg(uint8_t reg_addr, size_t length, uint8_t *data) {
    if (!length)
        return -1;
    uint8_t messeage[length + 1];
    messeage[0] = reg_addr;
    for (int i = 0; i < length; i++) {
        messeage[i + 1] = data[i];
    }
    if (i2c_write_blocking(i2c, ds3231_addr, messeage, (length + 1), false) == PICO_ERROR_GENERIC)
        return -1;
    return 0;
}

/**
 * @brief           Library function that takes an 8 bit unsigned integer and converts into
 *  Binary Coded Decimal number that can be written to DS3231 registers.
 *
 * @param[in] data  Number to be converted.
 * @return          Number in BCD form.
 */
uint8_t DS3231::bin_to_bcd(const uint8_t data) {
    uint8_t ones_digit = (uint8_t)(data % 10);
    uint8_t twos_digit = (uint8_t)(data - ones_digit) / 10;
    return ((twos_digit << 4) + ones_digit);
}

/**
 * @brief           Library function that takes a BCD number and converts it to an unsigned 8 bit integer.
 * 
 * @param[in] bcd   BCD number to be converted.
 * @return          Unsigned 8 bit integer.
 */
uint8_t DS3231::bcd_to_bin(const uint8_t bcd) {
    uint8_t ones_digit = (uint8_t)(bcd & 0x0F);
    uint8_t twos_digit = (uint8_t)(bcd >> 4);
    return (twos_digit * 10 + ones_digit);
}