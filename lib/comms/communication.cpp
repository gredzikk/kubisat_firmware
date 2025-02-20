#include "communication.h"

string outgoing;
uint8_t msgCount = 0;
long lastSendTime = 0;
long lastReceiveTime = 0;
long lastPrintTime = 0;
unsigned long interval = 0;


/**
 * @brief Initializes the LoRa radio module.
 * @return True if initialization was successful, false otherwise.
 * @details Sets the LoRa pins and attempts to begin LoRa communication at a specified frequency.
 *          Emits a CommsEvent::RADIO_INIT event on success or a CommsEvent::RADIO_ERROR event on failure.
 */
bool initialize_radio() {
    LoRa.set_pins(lora_cs_pin, lora_reset_pin, lora_irq_pin);
    long frequency = 433E6;
    bool initStatus = false;
    if (!LoRa.begin(frequency))
    {
        uart_print("LoRa init failed. Check your connections.", VerbosityLevel::WARNING);
        initStatus = false;
    } else {
        uart_print("LoRa initialized with frequency " + std::to_string(frequency), VerbosityLevel::INFO);
        initStatus = true;
    }

    EventEmitter::emit(EventGroup::COMMS, initStatus ? CommsEvent::RADIO_INIT : CommsEvent::RADIO_ERROR);

    return initStatus;
}

