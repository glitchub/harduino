// Stepper project
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Using UNO R3
#define F_CPU 16000000      // 16Mhz
#include "gpio.h"
#include "uno_r3.h"

// millisecond tick driver
#include "ticks.h"

// Stepper definitions
#define STEPPER_N GPIO02            // North aka A1
#define STEPPER_E GPIO03            // East aka B1
#define STEPPER_S GPIO04            // South aka A2
#define STEPPER_W GPIO05            // West aka B2
#define STEPPER_SLOW_HZ 500         // Start/end step frequency
#define STEPPER_FAST_HZ 1500        // Max step frequency
#define STEPPER_PHASES 8            // Steps per rotation (half-steps)
#include "stepper.h"
