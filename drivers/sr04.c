// SR04 ultrasonic ranging driver

// The module has TRIG input and ECHO output, normally both low. When the
// module sees TRIG go high for 10uS it transmits a burst of ultrasonic pulses
// and sets ECHO high. ECHO goes low when echo of the burst is received, or on
// timeout (about 38mS).

#if !defined(SR04_TRIG) || !defined(SR04_ECHO)
#error "Must define pins SR04_TRIG and SR04_ECHO"
#endif

#include "waituS.h"

void init_sr04(void)
{
    DDR(SR04_TRIG) |= BIT(SR04_TRIG); // trigger is an output
}

// 8-cycle loops per microsecond, works for 8Mhz and 16Mhz clocks
#define uS (F_CPU/8000000L)

// Trigger the sr04 and return echo pulse width converted to centimeters.
// Return -1 if pulse timed out (> 25 mS) or -2 if SR04 didn't respond. May
// block up to 25 mS. Do not call more often than once per 100mS.
int16_t get_sr04(void)
{
    uint16_t loops;
    PORT(SR04_TRIG) |= BIT(SR04_TRIG);
    waituS(10);
    PORT(SR04_TRIG) &= NOBIT(SR04_TRIG);
    // wait 1mS for ECHO to go high
    loops=1000*uS;
    while (!(PIN(SR04_ECHO) & BIT(SR04_ECHO))) { if (!--loops) return -2; __asm__ __volatile__ ("nop\n\tnop"); } // loop is 8 cycles
    // wait 25 mS for ECHO to go low
    loops=25000*uS;
    while ((PIN(SR04_ECHO) & BIT(SR04_ECHO))) { if (!--loops) return -1; __asm__ __volatile__ ("nop\n\tnop"); } // loop is 8 cycles
    // Round trip time is ~58 uS per centimeter
    return ((25000*uS)-loops)/(uS * 58);
}
