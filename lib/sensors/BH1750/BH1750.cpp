#include "BH1750.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <iostream>

BH1750::BH1750(uint8_t addr) : _i2c_addr(addr) {}

bool BH1750::begin(Mode mode) {
    write8(static_cast<uint8_t>(Mode::POWER_ON));
    write8(static_cast<uint8_t>(Mode::RESET));
    configure(mode);
    configure(BH1750::Mode::POWER_ON);
    uint8_t check = 0;
    uint8_t cmd = 0x10; // Continuously H-Resolution Mode
    if (i2c_write_blocking(i2c1, _i2c_addr, &cmd, 1, false) == 1) {
        std::cout << "BH1750 sensor found at 0x" << std::hex << (int)_i2c_addr << std::endl;
        return true;
    }
    return false;
}

void BH1750::configure(Mode mode) {
    uint8_t modeVal = static_cast<uint8_t>(mode);
    switch (mode) {
        case Mode::CONTINUOUS_HIGH_RES_MODE:
        case Mode::CONTINUOUS_HIGH_RES_MODE_2:
        case Mode::CONTINUOUS_LOW_RES_MODE:
        case Mode::ONE_TIME_HIGH_RES_MODE:
        case Mode::ONE_TIME_HIGH_RES_MODE_2:
        case Mode::ONE_TIME_LOW_RES_MODE:
            write8(modeVal);
            sleep_ms(10);
            break;
        default:
            printf("Invalid measurement mode\n");
            break;
    }
}

float BH1750::get_light_level() {
    uint8_t buffer[2];
    i2c_read_blocking(i2c_default, _i2c_addr, buffer, 2, false);
    uint16_t level = (buffer[0] << 8) | buffer[1];
    
    float lux = static_cast<float>(level) / 1.2f;
    return lux;
}

void BH1750::write8(uint8_t data) {
    uint8_t buf[1] = {data};
    i2c_write_blocking(i2c_default, _i2c_addr, buf, 1, false);
}