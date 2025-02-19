#ifndef HMC5883L_H
#define HMC5883L_H

#include "hardware/i2c.h"

class HMC5883L {
public:
    HMC5883L(i2c_inst_t* i2c, uint8_t address = 0x0D);
    bool init();
    bool read(int16_t& x, int16_t& y, int16_t& z);

private:
    i2c_inst_t* i2c;
    uint8_t address;

    bool write_register(uint8_t reg, uint8_t value);
    bool read_register(uint8_t reg, uint8_t* buffer, size_t length);
};

#endif