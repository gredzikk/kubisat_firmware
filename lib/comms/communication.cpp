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
    // Set pins
    LoRa.set_pins(LORA_DEFAULT_SS_PIN, LORA_DEFAULT_RESET_PIN, LORA_DEFAULT_DIO0_PIN);
    
    // Initialize at lower frequency for better range (depends on your region)
    // 433 MHz offers better penetration than 868/915 MHz
    if (!LoRa.begin(433E6)) {
        uart_print("LoRa initialization failed", VerbosityLevel::ERROR);
        EventEmitter::emit(EventGroup::COMMS, CommsEvent::RADIO_ERROR);
        return false;
    }
    
    // Optimize for long range
    LoRa.setSpreadingFactor(10);           // SF10 balances range and power
    LoRa.setSignalBandwidth(125E3);        // 125 kHz bandwidth
    LoRa.setCodingRate4(8);                // 4/8 coding rate
    LoRa.setPreambleLength(12);            // Extended preamble
    LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setSyncWord(0x69);                // Custom sync word
    LoRa.enableCrc();
    LoRa.setGain(5);                       // Higher gain (1-6)
    LoRa.setOCP(100);                      // OCP at 100mA
    
    // Register callback for packet reception
    LoRa.onReceive(on_receive);
    LoRa.receive();
    
    uart_print("LoRa radio initialized with long-range profile", VerbosityLevel::INFO);
    EventEmitter::emit(EventGroup::COMMS, CommsEvent::RADIO_INIT);
    
    return true;
}

void adjustLoRaParametersByLinkQuality() {
    int rssi = LoRa.rssi();
    float snr = LoRa.packetSnr();
    
    // Poor link quality, optimize for range
    if (rssi < -100 || snr < 5) {
        LoRa.setSpreadingFactor(12);
        LoRa.setSignalBandwidth(62.5E3);
        LoRa.setTxPower(20);
    }
    // Medium link quality, balance range and power
    else if (rssi < -85 || snr < 10) {
        LoRa.setSpreadingFactor(10);
        LoRa.setSignalBandwidth(125E3);
        LoRa.setTxPower(17);
    }
    // Good link quality, optimize for power saving
    else {
        LoRa.setSpreadingFactor(8);
        LoRa.setSignalBandwidth(250E3);
        LoRa.setTxPower(14);
    }
}

