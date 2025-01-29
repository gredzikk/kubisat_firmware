#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "PowerManager.h"

#include "ISensor.h"
#include "lib/BH1750/BH1750_WRAPPER.h"
#include "lib/BME280/BME280_WRAPPER.h"
#include "lib/DS3231/DS3231.h"
#include "lib/HMC5883L/HMC5883L_WRAPPER.h"
#include "lib/LoRa/LoRa-RP2040.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <chrono>
#include "communication.h"
#include "GPSData.h"
#include "sd_card.h"
#include "ff.h"

#include <iostream>
#include <map>
#include "commands.h"
#include "pin_config.h"
#include "storage.h"

uint32_t last_gps_time = 0;
GPSData gpsData;

char buffer[BUFFER_SIZE];
int bufferIndex = 0;

bool initSystems(i2c_inst_t *i2c_port) {
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    i2c_init(i2c_port, 400 * 1000);
    gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SCL);
    gpio_pull_up(I2C1_SDA);

    uart_init(GPS_UART, GPS_BAUD_RATE);
    gpio_set_function(GPS_TX_PIN, UART_FUNCSEL_NUM(GPS_UART, GPS_TX_PIN));
    gpio_set_function(GPS_RX_PIN, UART_FUNCSEL_NUM(GPS_UART, GPS_RX_PIN));

    if (true)
    {
        gpio_init(GPS_POWER_ENABLE);
        gpio_set_dir(GPS_POWER_ENABLE, GPIO_OUT);
        gpio_put(GPS_POWER_ENABLE, 1); // Set GPS_POWER_ENABLE pin high
    }

    std::cout << "GPIOS init done" << std::endl;

    for (int i = 5; i > 0; --i)
    {
        std::cout << "Program starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    return true;
}

int main()
{
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test02.txt";
    i2c_inst_t *i2c_port = I2C_PORT;
    initSystems(i2c_port);

    DS3231 systemClock(i2c_port);
    systemClock.setTime(0, 41, 20, 4, 14, 11, 2024);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    PowerManager powerManager(i2c_port);
    if (powerManager.initialize())
    {
        std::map<std::string, std::string> powerConfig = {
            {"operating_mode", "continuous"},
            {"averaging_mode", "16"},
        };
        powerManager.configure(powerConfig);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    bool radioInitSuccess = false;

    radioInitSuccess = initializeRadio();
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    lastReceiveTime = to_ms_since_boot(get_absolute_time());
    lastPrintTime = lastReceiveTime;

    bool sdTestResult = testSDCard();
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    if (radioInitSuccess)
    {
        LoRa.beginPacket();
        if (sdTestResult) {
            LoRa.print("System initialized successfully! SD test was performed.");
        } else {
            LoRa.print("System initialized successfully! SD test was skipped.");
        }
        LoRa.endPacket();
    }
    
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    for (int i = 5; i > 0; --i)
    {
        std::cout << "Main loop starts in " << i << " seconds..." << std::endl;
        gpio_put(PICO_DEFAULT_LED_PIN, (i%2==0));
        sleep_ms(1000);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (true)
    {
        uint8_t sec, min, hour, day, month;
        uint16_t year;
        std::string weekday;      

        long currentTime = to_ms_since_boot(get_absolute_time());

        int packetSize = LoRa.parsePacket();
        if (packetSize)
        {
            onReceive(packetSize);
        }

        if (currentTime - lastReceiveTime > 5000)
        {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "\n=== Power Manager Measurements ===\n";
            std::cout << "5V Rail Voltage     : " << powerManager.getVoltage5V() << " V\n";
            std::cout << "Battery Voltage     : " << powerManager.getVoltageBattery() << " V\n";
            std::cout << "Solar Charge Current: " << powerManager.getCurrentChargeSolar() << " mA\n";
            std::cout << "USB Charge Current  : " << powerManager.getCurrentChargeUSB() << " mA\n";
            std::cout << "Current Draw        : " << powerManager.getCurrentDraw() << " mA\n";

            systemClock.getTime(sec, min, hour, weekday, day, month, year);

            printf("%02d:%02d:%02d %s %02d.%02d.%d\n",
                   hour, min, sec, weekday.c_str(), day, month, year);

            logMessage("No messages received in the last 5 seconds.");
            lastReceiveTime = currentTime;
        }
    }

    return 0;
}

// BH1750    0X23
// INA3221   0X40
// BME280    0X76
// DS3231    0X86