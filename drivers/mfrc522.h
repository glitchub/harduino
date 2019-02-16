// Initialize MFRC522 RFID reader and return true, or false if chip fails to
// respond.
bool init_mfrc522(void);

// Given pointer to a 10 byte buffer, copy card UID to buffer and return 4, 7,
// or 10 to indicate the length. Return <= 0 if no card detected. 7- and
// 10-byte UIDs should be unique (barring intentional card duplication). 4-byte
// UIDs are not guaranteed to be unique. If a 4-byte UID starts with 0x08 then
// the other three bytes are randomly generated every time the card is read.
int8_t get_mfrc522(uint8_t *uid);
