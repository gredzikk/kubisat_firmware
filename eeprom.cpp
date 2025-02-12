bool writeEEPROM(uint16_t address, const uint8_t* data, size_t length) {
    uint8_t buffer[256]; // Buffer for address + data
    buffer[0] = (address >> 8) & 0xFF; // High byte of address
    buffer[1] = address & 0xFF;        // Low byte of address
    memcpy(buffer + 2, data, length);

    int ret = i2c_write_blocking(I2C_PORT, I2C_ADDR, buffer, length + 2, false);
    if (ret == PICO_ERROR_GENERIC) {
        uartPrint("Error: I2C write failed.");
        return false;
    }
    sleep_ms(5); // Wait for write cycle to complete
    return true;
}

void logToEEPROM(const std::string& message) {
    static uint16_t eeprom_write_address = 0;
    std::stringstream ss;
    ss << std::chrono::system_clock::now() << ": " << message << "\n";
    std::string logEntry = ss.str();

    if (!writeEEPROM(eeprom_write_address, (const uint8_t*)logEntry.c_str(), logEntry.length())) {
        uartPrint("Error: Failed to write to EEPROM.");
    }

    eeprom_write_address += logEntry.length();
    if (eeprom_write_address > 65535) { // Example: 64KB EEPROM
        eeprom_write_address = 0; // Wrap around
    }
}

std::string readEEPROMData(uint16_t startAddress, size_t length) {
    std::string data;
    uint8_t buffer[2];
    buffer[0] = (startAddress >> 8) & 0xFF;
    buffer[1] = startAddress & 0xFF;

    i2c_write_blocking(I2C_PORT, I2C_ADDR, buffer, 2, true); // Send address, no stop
    uint8_t readBuffer[256];
    int ret = i2c_read_blocking(I2C_PORT, I2C_ADDR, readBuffer, length, false);
     if (ret == PICO_ERROR_GENERIC) {
        uartPrint("Error: I2C read failed.");
        return "";
    }

    data.assign(readBuffer, readBuffer + length);
    return data;
}

std::string handleReadEEPROMLog(const std::string& param) {
    uint16_t startAddress = 0;
    size_t length = 1024; // Example: Read 1KB at a time
    std::string eepromData = readEEPROMData(startAddress, length);
    return eepromData;
}