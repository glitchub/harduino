// RFID demo project definitions

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

// Using UNO R3
#define F_CPU 16000000      // 16Mhz
#include "uno_r3.h"

// delay loop
#include "waituS.h"

// use threads
#include "threads.h"

// use millisecond ticks
#include "ticks.h"

// Serial transmit and receive pins are GPIO00 and GPIO01.
#define SERIAL_STDIO 1      // enable fprintf
#define SERIAL_TX_SIZE 60   // transmit buffer size
#define SERIAL_RX_SIZE 4    // receive buffer size
#include "serial.h"

// SPI configuration
#define SPI_ORDER 0         // send MSB first
#define SPI_MODE 0          // clock is active high, sample on leading edge
#define SPI_CLOCK 3         // clock F_CPU/128 (125Khz)
#include "spi.h"

// RFID configuration
#define MFRC522_RST GPIO09
#include "mfrc522.h"
