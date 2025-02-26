#include "pin_config.h"

// LoRa constants
const int lora_cs_pin = 17;          // LoRa radio chip select
const int lora_reset_pin = 22;       // LoRa radio reset
const int lora_irq_pin = 28;         // LoRa hardware interrupt pin

uint8_t lora_address_local = 37;     // address of this device
uint8_t lora_address_remote = 21; 