// Read or write an 8-bit value at eeprom offset.
void write_eeprom(uint16_t offset, uint8_t byte);
uint8_t read_eeprom(uint16_t offset);

// Write 4-bit param to specified ffset, with a checksum
void write_eeparam(uint16_t offset, uint8_t param);

// Read 4-bit param from offset, if valid checksum set *param and return true,
// else leave *param alone and return false
bool read_eeparam(uint16_t offset, uint8_t *param);
