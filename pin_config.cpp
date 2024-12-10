#include "pin_config.h"

// LoRa constants
const int csPin = 17;          // LoRa radio chip select
const int resetPin = 15;       // LoRa radio reset
const int irqPin = 14;         // LoRa hardware interrupt pin

uint8_t localAddress = 37;     // address of this device
uint8_t destination = 21; 