// Thread demo project definitions

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/sleep.h>

// Using UNO R3
#define F_CPU 16000000      // 16Mhz
#include "uno_r3.h"

// use threads
#include "threads.h"

// use millisecond ticks
#include "ticks.h"

// Serial transmit and receive pins are GPIO00 and GPIO01.
#define SERIAL_STDIO 1      // enable fprintf
#define SERIAL_TX_SIZE 60   // transmit buffer size
#define SERIAL_RX_SIZE 4    // receive buffer size
#include "serial.h"

// DHT11 temp/humidity sensor
#define DHT11_IO GPIO02     // requires one i/o pin
#include "dht11.h"
