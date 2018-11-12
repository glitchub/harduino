// Demo project definitions

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

// Using UNO R3
#define F_CPU 16000000      // 16Mhz
#include "uno_r3.h"

// millisecond tick driver
#include "ticks.h"

// Serial transmit and receive pins are GPIO00 and GPIO01.
#define SERIAL_STDIO 1      // enable fprintf
#define SERIAL_TX_SIZE 60   // transmit buffer size
#define SERIAL_RX_SIZE 4    // receive buffer size
#include "serial.h"

// NEC IR, detector attaches to ICP1 aka GPIO08.
#define NEC_IDLE 1          // Detector output high when not receiving a pulse
#include "nec.h"

// LCD module
#define LCD_STDIO           // enable file handle for fprintf
#define LCD_D4 GPIO03       // Outputs
#define LCD_D5 GPIO04
#define LCD_D6 GPIO05
#define LCD_D7 GPIO06
#define LCD_RS GPIO07
#define LCD_E  GPIO02
#include "lcd.h"

// DHT11 temp/humidity sensor
#define DHT11_IO GPIO12     // requires one i/o pin
#include "dht11.h"

// SR04 ultrasonic range module
#define SR04_TRIG GPIO10    // requires one output
#define SR04_ECHO GPIO09    // and one input
#include "sr04.h"

