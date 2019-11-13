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

#include "gpio.h"

#include BOARD

// printf but with the format string in program space
#define pprintf(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
