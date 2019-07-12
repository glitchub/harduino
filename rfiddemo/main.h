// RFID demo project definitions

#define BOARD "uno_r3.h"
#define TICKMS 4

// SPI definitions
#define SPI_ORDER 0         // send MSB first
#define SPI_MODE 0          // clock is active high, sample on leading edge
#define SPI_CLOCK 3         // clock MHZ/128 (125Khz)

// RFID definitions
#define MFRC522_RST GPIO09
