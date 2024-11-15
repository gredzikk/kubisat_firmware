#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "ISensor.h"
#include "lib/BH1750/BH1750_WRAPPER.h"
#include "lib/BME280/BME280_WRAPPER.h"
#include "lib/DS3231/DS3231.h"
#include "lib/INA3221/INA3221_WRAPPER.h"
#include "lib/LoRa/LoRa-RP2040.h"
#include <iostream>
#include <iomanip>


#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define I2C_PORT i2c0

int main()
{
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    i2c_inst_t* i2c = i2c0;
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);    

    for (int i = 10; i > 0; --i) {
        std::cout << "Program starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    auto& sensorManager = SensorWrapper::getInstance();

    sensorManager.initSensor(SensorType::LIGHT, i2c);
    sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);
    sensorManager.initSensor(SensorType::POWER, i2c);

    std::map<std::string, std::string> powerConfig = {
        {"operating_mode", "continuous"},  
        {"averaging_mode", "16"},         
    };

    sensorManager.configureSensor(SensorType::POWER, powerConfig);

    DS3231 systemClock(i2c);    
    DateTime systemTime;
    systemTime.year = 2024;
    systemTime.month = 11;
    systemTime.day = 15;
    systemTime.weekday = Weekday::FRI;
    systemTime.hour = 8;
    systemTime.min = 25;
    systemTime.sec = 0;

    systemClock.setTime(systemTime);

    for (int i = 5; i > 0; --i) {
        std::cout << "Main loop starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    while(true) {
        float lightLevel = sensorManager.readSensorData(SensorType::LIGHT, DataType::LIGHT_LEVEL);
        float temperature = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::TEMPERATURE);
        float humidity = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::HUMIDITY);
        float pressure = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::PRESSURE);

        float voltage_ch1 = sensorManager.readSensorData(SensorType::POWER, DataType::VOLTAGE_BATTERY);
        float voltage_ch2 = sensorManager.readSensorData(SensorType::POWER, DataType::VOLTAGE_5V_OUT);

        float current_ch1 = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_CHARGE_USB);
        float current_ch2 = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_DRAW);
        float current_ch3 = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_CHARGE_SOLAR);
        
        if(systemClock.getTime(systemTime)) {
            std::cout << systemTime.year << "-" << systemTime.month << "-" << systemTime.day << "-" << DS3231::weekdayToString(systemTime.weekday) << " " << systemTime.hour << ":" << systemTime.min << ":" << systemTime.sec << std::endl;
        }

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Light: " << lightLevel << " lux" << std::endl;
        std::cout << "Temperature: " << temperature << " C" << std::endl; 
        std::cout << "Humidity: " << humidity << " %" << std::endl;
        std::cout << "Pressure: " << pressure << " hPa" << std::endl;
        
        std::cout << "Voltages: 1: " << voltage_ch1 << "V, 2: " << voltage_ch2 << "V" << std::endl;
        std::cout << "Current: 1: " << current_ch1 << "mA, 2: " << current_ch2 << "mA, 3: " << current_ch3 << "mA" << std::endl;

        sleep_ms(1000);
    }

    return 0;
}


// BH1750    0X23
// INA3221   0X40
// BME280    0X76
// DS3231    0X86