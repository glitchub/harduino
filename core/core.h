// core header files
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include BOARD
#include "gpio.h"
#include "ticks.h"

// Default to four-millisecond tick resolution. (Higher numbers == less
// interrupts == lower idle current)
#ifndef TICKMS
#define TICKMS 4
#endif

// Default to 128 mS watchdog reset.
#ifndef WATCHDOG
#define WATCHDOG 128
#endif

// printf but with the format string in program space
#define pprintf(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
