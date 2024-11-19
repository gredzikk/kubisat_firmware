// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "hardware/i2c.h"
#include "hardware/uart.h"

// DEBUG UART configuration
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 12
#define UART_RX_PIN 13

// I2C configuration
#define I2C_PORT i2c0

// LoRa constants
const int csPin = 17;      // LoRa radio chip select
const int resetPin = 15;   // LoRa radio reset
const int irqPin = 14;     // LoRa radio interrupt pin

const uint8_t localAddress = 37; // Address of this device
const uint8_t destination = 21;  // Destination to send to

// GPS configuration
#define GPS_UART uart1
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 4
#define GPS_RX_PIN 5
#define BUFFER_SIZE 85  // NMEA sentences are usually under 85 chars

#endif // CONFIG_H