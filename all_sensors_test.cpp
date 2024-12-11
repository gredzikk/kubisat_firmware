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
#include "sd_card.h"
#include "ff.h"

#include <iostream>
#include <map>
#include "commands.h"
#include "pin_config.h"  // Add this line

uint32_t last_gps_time = 0;
GPSData gpsData;

// LoRa contants
using std::string;

      // change for your board; must be a hardware interrupt pin

string outgoing;              // outgoing message

uint8_t msgCount = 0;            // count of outgoing messages

      // destination to send to

long lastSendTime = 0;        // last send time
long unsigned int interval = 2000; // interval between sends
long lastReceiveTime = 0;     // last receive time

// LoRa methods
void logMessage(const string& message) {
	uint32_t timestamp = to_ms_since_boot(get_absolute_time());
	printf("[%lu ms] %s\n", timestamp, message.c_str());
}

void sendMessage(string outgoing) {
	int n = outgoing.length();
	char send[n+1];
	strcpy(send,outgoing.c_str());
	logMessage("Sat to ground: " + string(send) + " [" + std::to_string(n) + "]");
	LoRa.beginPacket();                   // start packet
	LoRa.write(destination);              // add destination address
	LoRa.write(localAddress);             // add sender address
	LoRa.write(msgCount);                 // add message ID
	LoRa.write(n+1);                      // add payload length
	LoRa.print(send);                     // add payload
	LoRa.endPacket();                     // finish packet and send it
	msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  // read packet header uint8_ts:
  int recipient = LoRa.read();          // recipient address
  uint8_t sender = LoRa.read();         // sender address
  uint8_t incomingMsgId = LoRa.read();  // incoming msg ID
  uint8_t incomingLength = LoRa.read(); // incoming msg length

  string incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length() + 1) {   // check length for error
    printf("error: message length does not match length\n");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    printf("This message is not for me.\n");
    return;                             // skip rest of function
  }

  logMessage("Received from: 0x" + std::to_string(sender));
  logMessage("Sent to: 0x" + std::to_string(recipient));
  logMessage("Message ID: " + std::to_string(incomingMsgId));
  logMessage("Message length: " + std::to_string(incomingLength));
  logMessage("Message: " + incoming);
  logMessage("RSSI: " + std::to_string(LoRa.packetRssi()));
  logMessage("Snr: " + std::to_string(LoRa.packetSnr()));

  sleep_ms(500);
  handleCommand(incoming);

  // Update last receive time
  lastReceiveTime = to_ms_since_boot(get_absolute_time());
}

char buffer[BUFFER_SIZE];
int bufferIndex = 0;

//GPS methods
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

bool testSDCard() {
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test02.txt";

    printf("\r\nSD card test. Press 'enter' to start or 's' to skip.\n");
    while (true) {
        buf[0] = getchar();
        if (buf[0] == 's') {
            printf("Skipping SD card test.\n");
            return false;
        }
        if ((buf[0] == '\r') || (buf[0] == '\n')) {
            break;
        }
    }

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card. Press 's' to skip.\n");
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d). Press 's' to skip.\n", fr);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Open file for writing
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d). Press 's' to skip.\n", fr);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Write something to file
    ret = f_printf(&fil, "This is another test\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d). Press 's' to skip.\n", ret);
        f_close(&fil);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }
    ret = f_printf(&fil, "of writing to an SD card.\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d). Press 's' to skip.\n", ret);
        f_close(&fil);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d). Press 's' to skip.\n", fr);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Open file for reading
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d). Press 's' to skip.\n", fr);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Print every line in file over serial
    printf("Reading from file '%s':\n", filename);
    printf("---\n");
    while (f_gets(buf, sizeof(buf), &fil)) {
        printf("%s", buf);
    }
    printf("\n---\n");

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d). Press 's' to skip.\n", fr);
        while (true) {
            buf[0] = getchar();
            if (buf[0] == 's') {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }

    // Unmount drive
    f_unmount("0:");
    printf("SD card test completed successfully.\n");
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

    // sensorManager.initSensor(SensorType::LIGHT, i2c);
    // sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);
    sensorManager.initSensor(SensorType::POWER, i2c);
    // sensorManager.initSensor(SensorType::MAGNETOMETER, i2c);

    std::map<std::string, std::string> powerConfig = {
        {"operating_mode", "continuous"},  
        {"averaging_mode", "16"},         
    };

    sensorManager.configureSensor(SensorType::POWER, powerConfig);

    DS3231 systemClock(i2c);    

    systemClock.setTime(0,41,20,4,14,11,2024);

    //LoRa init
    LoRa.setPins(csPin, resetPin, irqPin);

    if (!LoRa.begin(500E6)) {
        logMessage("LoRa init failed. Check your connections.");
        while (true);
    }

    logMessage(" init succeeded.");

    lastReceiveTime = to_ms_since_boot(get_absolute_time());
    for (int i = 5; i > 0; --i) {
        std::cout << "Main loop starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    testSDCard();
    
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

        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            onReceive(packetSize);
        }

        long currentTime = to_ms_since_boot(get_absolute_time());
        if (currentTime - lastReceiveTime > 5000) {
            logMessage("No messages received in the last 5 seconds.");
            lastReceiveTime = currentTime;
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