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
bool initializeRadio() {
    LoRa.setPins(csPin, resetPin, irqPin);
    long frequency = 433E6;
    bool initStatus = false;
    if (!LoRa.begin(frequency))
    {
        uartPrint("LoRa init failed. Check your connections.");
        initStatus = false;
    } else {
        uartPrint("LoRa initialized with frequency " + std::to_string(frequency));
        initStatus = true;
    }

    EventEmitter::emit(EventGroup::COMMS, initStatus ? CommsEvent::RADIO_INIT : CommsEvent::RADIO_ERROR);

    return initStatus;
}

