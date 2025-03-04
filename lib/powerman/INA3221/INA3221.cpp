#include "INA3221.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <iostream>
#include "pin_config.h"
#include "utils.h"
#include <sstream>


/**
 * @file INA3221.cpp
 * @brief Implementation of the INA3221 power monitor driver
 * @details This file contains the implementation for the INA3221 triple-channel power monitor,
 *          providing functionality for voltage, current, and power monitoring with alert capabilities.
 */


/**
 * @defgroup INA3221 INA3221 Power Monitor
 * @{
 *   @defgroup INA3221_Config Configuration Functions
 *   Functions for configuring the INA3221 device
 *   @{
 *   @}
 *   
 *   @defgroup INA3221_Measure Measurement Functions
 *   Functions for reading voltage, current and power measurements
 *   @{
 *   @}
 *   
 *   @defgroup INA3221_Alert Alert Functions
 *   Functions for configuring and reading alert conditions
 *   @{
 *   @}
 * @}
 */


/**
 * @ingroup INA3221_Config
 * @brief Constructor for INA3221 class
 * @param addr I2C address of the device
 * @param i2c Pointer to I2C instance
 */
INA3221::INA3221(ina3221_addr_t addr, i2c_inst_t* i2c)
    : _i2c_addr(addr), _i2c(i2c) {}


/**
 * @ingroup INA3221_Config
 * @brief Initialize the INA3221 device
 * @return true if initialization successful, false otherwise
 * @details Sets up shunt resistors, filter resistors, and verifies device IDs
 */
bool INA3221::begin() {
    uart_print("INA3221 initializing...", VerbosityLevel::DEBUG);

    _shuntRes[0] = 10;
    _shuntRes[1] = 10;
    _shuntRes[2] = 10;

    _filterRes[0] = 10;
    _filterRes[1] = 10;
    _filterRes[2] = 10;

    uint16_t manuf_id = get_manufacturer_id();
    uint16_t die_id = get_die_id();
    std::stringstream ss;
    ss << "INA3221 Manufacturer ID: 0x" << std::hex << manuf_id 
              << ", Die ID: 0x" << die_id;
    uart_print(ss.str(), VerbosityLevel::INFO);

    if (manuf_id == 0x5449 && die_id == 0x3220) { 
       uart_print("INA3221 found and initialized.", VerbosityLevel::DEBUG);
        return true;
    } else {
        uart_print("INA3221 initialization failed. Incorrect IDs.", VerbosityLevel::ERROR);
        return false;
    }

}


/**
 * @ingroup INA3221_Config
 * @brief Reset the INA3221 to default settings
 * @details Performs a software reset of the device by setting the reset bit
 */
void INA3221::reset(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.reset = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Get the manufacturer ID of the device
 * @return 16-bit manufacturer ID (should be 0x5449)
 */
uint16_t INA3221::get_manufacturer_id() {
    uint16_t id = 0;
    _read(INA3221_REG_MANUF_ID, &id);
    return id;
}


/**
 * @ingroup INA3221_Config
 * @brief Get the die ID of the device
 * @return 16-bit die ID (should be 0x3220)
 */
uint16_t INA3221::get_die_id() {
    uint16_t id = 0;
    _read(INA3221_REG_DIE_ID, &id);
    return id;
}


/**
 * @ingroup INA3221_Config
 * @brief Read a register from the device
 * @param reg Register address to read
 * @return 16-bit value read from the register
 */
uint16_t INA3221::read_register(ina3221_reg_t reg){
    uint16_t val = 0;
    _read(reg, &val);
    return val;
}


//configure

/**
 * @ingroup INA3221_Config
 * @brief Set device to power-down mode
 * @details Disables bus voltage and continuous measurements
 */
void INA3221::set_mode_power_down(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 0;
    conf_reg.mode_continious_en =0 ;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Set device to continuous measurement mode
 * @details Enables continuous measurement of bus voltage and shunt voltage
 */
void INA3221::set_mode_continuous(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_continious_en =1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Set device to triggered measurement mode
 * @details Disables continuous measurements, requiring manual triggers
 */
void INA3221::set_mode_triggered(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_continious_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Enable shunt voltage measurements
 */
void INA3221::set_shunt_measurement_enable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_shunt_en = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Disable shunt voltage measurements
 */
void INA3221::set_shunt_measurement_disable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_shunt_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Enable bus voltage measurements
 */
void INA3221::set_bus_measurement_enable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Disable bus voltage measurements
 */
void INA3221::set_bus_measurement_disable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Set the averaging mode for measurements
 * @param mode Number of samples to average
 */
void INA3221::set_averaging_mode(ina3221_avg_mode_t mode) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.avg_mode = mode;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Set bus voltage conversion time
 * @param convTime Conversion time setting
 */
void INA3221::set_bus_conversion_time(ina3221_conv_time_t convTime) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.bus_conv_time = convTime;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


/**
 * @ingroup INA3221_Config
 * @brief Set shunt voltage conversion time
 * @param convTime Conversion time setting
 */
void INA3221::set_shunt_conversion_time(ina3221_conv_time_t convTime) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.shunt_conv_time = convTime;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}


//get measurement 
/**
 * @ingroup INA3221_Measure
 * @brief Get shunt voltage for a specific channel
 * @param channel Channel number (1-3)
 * @return Shunt voltage in microvolts (µV)
 */
int32_t INA3221::get_shunt_voltage(ina3221_ch_t channel) {
    int32_t res;
    ina3221_reg_t reg;
    uint16_t val_raw = 0;

    switch(channel){
        case INA3221_CH1:
            reg = INA3221_REG_CH1_SHUNTV;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_SHUNTV;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_SHUNTV;
            break;
    }

    _read(reg, &val_raw);

    res = (int16_t) (val_raw >> 3);
    res *= SHUNT_VOLTAGE_LSB_UV; 

    return res;
}


/**
 * @ingroup INA3221_Measure
 * @brief Get current for a specific channel
 * @param channel Channel number (1-3)
 * @return Current in milliamps (mA)
 */
float INA3221::get_current_ma(ina3221_ch_t channel) {
    int32_t shunt_uV = 0;
    float current_A = 0;

    shunt_uV = get_shunt_voltage(channel);
    current_A = shunt_uV / (int32_t)_shuntRes[channel];
    return current_A;
}


/**
 * @ingroup INA3221_Measure
 * @brief Get bus voltage for a specific channel
 * @param channel Channel number (1-3)
 * @return Voltage in volts (V)
 */
float INA3221::get_voltage(ina3221_ch_t channel) {
    float voltage_V = 0.0;
    ina3221_reg_t reg;
    uint16_t val_raw = 0;

    switch(channel){
        case INA3221_CH1:
            reg = INA3221_REG_CH1_BUSV;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_BUSV;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_BUSV;
            break;
    }

    _read(reg, &val_raw);
    voltage_V = val_raw / 1000.0;
    return voltage_V;
}


// alerts
/**
 * @ingroup INA3221_Alert
 * @brief Set warning alert voltage threshold for a channel
 * @param channel Channel number (1-3)
 * @param voltage_v Voltage threshold in volts
 */
void INA3221::set_warn_alert_limit(ina3221_ch_t channel, float voltage_v) {
    ina3221_reg_t reg;
    uint16_t val = (uint16_t)(voltage_v * 1000); // Convert V to mV

    switch(channel) {
        case INA3221_CH1:
            reg = INA3221_REG_CH1_WARNING_ALERT_LIM;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_WARNING_ALERT_LIM;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_WARNING_ALERT_LIM;
            break;
    }
    _write(reg, &val);
}


/**
 * @ingroup INA3221_Alert
 * @brief Set critical alert voltage threshold for a channel
 * @param channel Channel number (1-3)
 * @param voltage_v Voltage threshold in volts
 */
void INA3221::set_crit_alert_limit(ina3221_ch_t channel, float voltage_v) {
    ina3221_reg_t reg;
    uint16_t val = (uint16_t)(voltage_v * 1000); // Convert V to mV

    switch(channel) {
        case INA3221_CH1:
            reg = INA3221_REG_CH1_CRIT_ALERT_LIM;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_CRIT_ALERT_LIM;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_CRIT_ALERT_LIM;
            break;
    }
    _write(reg, &val);
}


/**
 * @ingroup INA3221_Alert
 * @brief Set power valid voltage range
 * @param voltage_upper_v Upper voltage threshold in volts
 * @param voltage_lower_v Lower voltage threshold in volts
 */
void INA3221::set_power_valid_limit(float voltage_upper_v, float voltage_lower_v) {
    uint16_t val;
    
    val = (uint16_t)(voltage_upper_v * 1000);
    _write(INA3221_REG_PWR_VALID_HI_LIM, &val);
    
    val = (uint16_t)(voltage_lower_v * 1000);
    _write(INA3221_REG_PWR_VALID_LO_LIM, &val);
}


/**
 * @ingroup INA3221_Alert
 * @brief Enable all alert functions
 * @details Enables warning alerts, critical alerts, and power valid alerts for all channels
 */
void INA3221::enable_alerts() {
    masken_reg_t masken;
    _read(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
    
    masken.warn_alert_ch1 = 1;
    masken.warn_alert_ch2 = 1;
    masken.warn_alert_ch3 = 1;
    masken.crit_alert_ch1 = 1;
    masken.crit_alert_ch2 = 1;
    masken.crit_alert_ch3 = 1;
    masken.pwr_valid_alert = 1;
    
    _write(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
}


/**
 * @ingroup INA3221_Alert
 * @brief Get warning alert status for a channel
 * @param channel Channel number (1-3)
 * @return true if warning alert is active, false otherwise
 */
bool INA3221::get_warn_alert(ina3221_ch_t channel) {
    masken_reg_t masken;
    _read(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
    
    switch(channel) {
        case INA3221_CH1: return masken.warn_alert_ch1;
        case INA3221_CH2: return masken.warn_alert_ch2;
        case INA3221_CH3: return masken.warn_alert_ch3;
        default: return false;
    }
}


/**
 * @ingroup INA3221_Alert
 * @brief Get critical alert status for a channel
 * @param channel Channel number (1-3)
 * @return true if critical alert is active, false otherwise
 */
bool INA3221::get_crit_alert(ina3221_ch_t channel) {
    masken_reg_t masken;
    _read(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
    
    switch(channel) {
        case INA3221_CH1: return masken.crit_alert_ch1;
        case INA3221_CH2: return masken.crit_alert_ch2;
        case INA3221_CH3: return masken.crit_alert_ch3;
        default: return false;
    }
}


/**
 * @ingroup INA3221_Alert
 * @brief Get power valid alert status
 * @return true if power valid alert is active, false otherwise
 */
bool INA3221::get_power_valid_alert() {
    masken_reg_t masken;
    _read(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
    return masken.pwr_valid_alert;
}


/**
 * @ingroup INA3221_Alert
 * @brief Set alert latch mode
 * @param enable true to enable alert latching, false for transparent alerts
 */
void INA3221::set_alert_latch(bool enable) {
    masken_reg_t masken;
    _read(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
    masken.warn_alert_latch_en = enable;
    masken.crit_alert_latch_en = enable;
    _write(INA3221_REG_MASK_ENABLE, (uint16_t*)&masken);
}


// private
/**
 * @brief Read a 16-bit register from the device
 * @param reg Register address
 * @param val Pointer to store the read value
 * @private
 */
void INA3221::_read(ina3221_reg_t reg, uint16_t *val) {
    uint8_t reg_buf = reg;
    uint8_t data[2];

    int ret = i2c_write_blocking(MAIN_I2C_PORT, _i2c_addr, &reg_buf, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write register address to I2C device." << std::endl;
        return;
    }

    ret = i2c_read_blocking(MAIN_I2C_PORT, _i2c_addr, data, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to read data from I2C device." << std::endl;
        return;
    }

    *val = (data[0] << 8) | data[1];
}


/**
 * @brief Write a 16-bit value to a register
 * @param reg Register address
 * @param val Pointer to the value to write
 * @private
 */
void INA3221::_write(ina3221_reg_t reg, uint16_t *val) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (*val >> 8) & 0xFF; // MSB
    buf[2] = (*val) & 0xFF;      // LSB

    int ret = i2c_write_blocking(MAIN_I2C_PORT, _i2c_addr, buf, 3, false);
    if (ret != 3) {
        std::cerr << "Failed to write data to I2C device." << std::endl;
    }
}
