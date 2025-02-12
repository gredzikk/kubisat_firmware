// pin_config.h
#include <stdint.h>

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

//DEBUG uart
#define DEBUG_UART_PORT uart0
#define DEBUG_UART_BAUD_RATE 115200

#define DEBUG_UART_TX_PIN 0
#define DEBUG_UART_RX_PIN 1

#define MAIN_I2C_PORT i2c1
#define MAIN_I2C_SDA_PIN 6
#define MAIN_I2C_SCL_PIN 7

// GPS configuration
#define GPS_UART_PORT uart1
#define GPS_UART_BAUD_RATE 115200
#define GPS_UART_TX_PIN 8
#define GPS_UART_RX_PIN 9
#define GPS_POWER_ENABLE_PIN 14

#define BUFFER_SIZE 85  // NMEA sentences are usually under 85 chars

// SPI configuration for SD card
#define SD_SPI_PORT spi1
#define SD_MISO_PIN 12
#define SD_MOSI_PIN 11
#define SD_SCK_PIN 10
#define SD_CS_PIN 13
#define SD_CARD_DETECT_PIN 28

#define SX1278_MISO 16
#define SX1278_CS   17
#define SX1278_SCK  18
#define SX1278_MOSI 19

#define SPI_PORT spi0
#define READ_BIT 0x80

#define LORA_DEFAULT_SPI           spi0
#define LORA_DEFAULT_SPI_FREQUENCY 8E6
#define LORA_DEFAULT_SS_PIN        17
#define LORA_DEFAULT_RESET_PIN     22
#define LORA_DEFAULT_DIO0_PIN      20

#define PA_OUTPUT_RFO_PIN          11
#define PA_OUTPUT_PA_BOOST_PIN     12



// LoRa constants - declare as extern
extern const int csPin;      // LoRa radio chip select
extern const int resetPin;   // LoRa radio reset  
extern const int irqPin;     // LoRa hardware interrupt pin
extern uint8_t localAddress; // address of this device
extern uint8_t destination;  // destination to send to


#endif // PIN_CONFIG_H