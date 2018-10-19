// Stepper project
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Using UNO R3
#define F_CPU 16000000      // 16Mhz
#include "uno_r3.h"     

// inline waituS and waitmS functions
#include "wait.h"

// millisecond tick driver
#include "ticks.h"

// Serial transmit and receive pins are GPIO00 and GPIO01.
#define SERIAL_STDIO 1      // enable fprintf
#define SERIAL_TX_SIZE 60   // transmit buffer size
#include "serial.h"

#define STEPPER_N GPIO02            // orange == A1
#define STEPPER_E GPIO03            // yellow == B1
#define STEPPER_S GPIO04            // pink == A2
#define STEPPER_W GPIO05            // blue == B2
#define STEPPER_SLOW_HZ 500         // start and end frequency
#define STEPPER_FAST_HZ 1500        // max frequency
#define STEPPER_PHASES 8            // steps per rotation  
#include "stepper.h"
