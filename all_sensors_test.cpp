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
#include "lib/HMC5883L/HMC5883L_WRAPPER.h"
#include "lib/LoRa/LoRa-RP2040.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <chrono>

#include "GPSData.h"

uint32_t last_gps_time = 0;
GPSData gpsData;

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define I2C_PORT i2c0

#define GPS_UART uart1
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 8
#define GPS_RX_PIN 9
#define BUFFER_SIZE 85  // NMEA sentences are usually under 85 chars

char buffer[BUFFER_SIZE];
int bufferIndex = 0;

void setupGPS() {
    uart_init(GPS_UART, GPS_BAUD_RATE);
    gpio_set_function(GPS_TX_PIN, UART_FUNCSEL_NUM(GPS_UART, GPS_TX_PIN));
    gpio_set_function(GPS_RX_PIN, UART_FUNCSEL_NUM(GPS_UART, GPS_RX_PIN));
}

void printSensorData(float lightLevel, float temperature, float humidity, float pressure,
                    float voltage_ch1, float voltage_ch2, float current_charge_usb,
                    float current_charge_solar, float current_draw,
                    float magnetic_1, float magnetic_2, float magnetic_3,
                    DS3231& systemClock) {
    
    uint8_t sec, min, hour, day, month;
    uint16_t year;
    std::string weekday;

    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    float seconds_since_gps = (current_time - last_gps_time) / 1000.0f;

    std::cout << "\n=== Sensor Data (GPS age: " << std::fixed << std::setprecision(1) 
              << seconds_since_gps << "s) ===" << std::endl;

    if(systemClock.getTime(sec, min, hour, weekday, day, month, year)) {
        printf("%02d:%02d:%02d %s %02d.%02d.%d\n", 
               hour, min, sec, weekday.c_str(), day, month, year);
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Light: " << lightLevel << " lux" << std::endl;
    std::cout << "Temperature: " << temperature << " C" << std::endl; 
    std::cout << "Humidity: " << humidity << " %" << std::endl;
    std::cout << "Pressure: " << pressure << " hPa" << std::endl;
    std::cout << "Voltages: 1: " << voltage_ch1 << "V, 2: " << voltage_ch2 << "V" << std::endl;
    std::cout << "Charge USB: " << current_charge_usb << "mA, Charge SOLAR: " << current_charge_solar << "mA" << std::endl;
    std::cout << "Total current draw: " << current_draw << "mA" << std::endl;
    std::cout << "Mag X: " << magnetic_1 << ", mag y: " << magnetic_2 << ", mag z: " << magnetic_3 << std::endl;
}

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

    setupGPS();

    for (int i = 5; i > 0; --i) {
        std::cout << "Program starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    auto& sensorManager = SensorWrapper::getInstance();

    sensorManager.initSensor(SensorType::LIGHT, i2c);
    sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);
    sensorManager.initSensor(SensorType::POWER, i2c);
    sensorManager.initSensor(SensorType::MAGNETOMETER, i2c);

    std::map<std::string, std::string> powerConfig = {
        {"operating_mode", "continuous"},  
        {"averaging_mode", "16"},         
    };

    sensorManager.configureSensor(SensorType::POWER, powerConfig);

    DS3231 systemClock(i2c);    

    systemClock.setTime(0,41,20,4,14,11,2024);

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

        float current_charge_usb = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_CHARGE_USB);
        float current_draw = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_DRAW);
        float current_charge_solar = sensorManager.readSensorData(SensorType::POWER, DataType::CURRENT_CHARGE_SOLAR);
        
        float magnetic_1 = sensorManager.readSensorData(SensorType::MAGNETOMETER, DataType::MAG_FIELD_X);
        float magnetic_2 = sensorManager.readSensorData(SensorType::MAGNETOMETER, DataType::MAG_FIELD_Y);
        float magnetic_3 = sensorManager.readSensorData(SensorType::MAGNETOMETER, DataType::MAG_FIELD_Z);

        uint8_t sec, min, hour, day, month;
        uint16_t year;
        std::string weekday;

        while (uart_is_readable(GPS_UART)) {
        char c = uart_getc(GPS_UART);
        
        if (c == '$') {  // Start of new NMEA sentence
            bufferIndex = 0;
            buffer[bufferIndex++] = c;
        }
        else if (c == '\n') {  // End of sentence
            if (bufferIndex < BUFFER_SIZE - 1) {
                buffer[bufferIndex++] = c;
                buffer[bufferIndex] = '\0';
                // Print complete NMEA sentence
                printf("%s", buffer);
            }
            bufferIndex = 0;
        }
        else if (bufferIndex < BUFFER_SIZE - 1) {
            buffer[bufferIndex++] = c;
        }
    }

        sleep_ms(100);
    }

    return 0;
}


// BH1750    0X23
// INA3221   0X40
// BME280    0X76
// DS3231    0X86