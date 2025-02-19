#include "HMC5883L.h"

HMC5883L::HMC5883L(i2c_inst_t* i2c, uint8_t address) : i2c(i2c), address(address) {}

bool HMC5883L::init() {
    // Continuous measurement mode, 15Hz data output rate
    if (!write_register(0x00, 0x70)) return false;
    if (!write_register(0x01, 0x20)) return false;
    if (!write_register(0x02, 0x00)) return false;
    return true;
}

bool HMC5883L::read(int16_t& x, int16_t& y, int16_t& z) {
    uint8_t buffer[6];
    if (!read_register(0x03, buffer, 6)) return false;

    x = (buffer[0] << 8) | buffer[1];
    z = (buffer[2] << 8) | buffer[3];
    y = (buffer[4] << 8) | buffer[5];

    if (x > 32767) x -= 65536;
    if (y > 32767) y -= 65536;
    if (z > 32767) z -= 65536;

    return true;
}

bool HMC5883L::write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    return i2c_write_blocking(i2c, address, buffer, 2, false) == 2;
}

bool HMC5883L::read_register(uint8_t reg, uint8_t* buffer, size_t length) {
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) return false;
    return i2c_read_blocking(i2c, address, buffer, length, false) == length;
}