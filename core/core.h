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

#include "gpio.h"
// XXX use BOARD macro
#include "uno_r3.h"
#include "ticks.h"
#include "serial.h"
#include "command.h"
#include "threads.h"
