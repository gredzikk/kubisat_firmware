uint32_t calculate_checksum(const uint8_t* data, size_t length);
void send_data_block(const uint8_t* data, size_t length);
bool receive_ack();