// SR04 ultrasonic ranging driver

// The module has TRIG input and ECHO output, normally both low. When the
// module sees TRIG go high for 10uS it transmits a burst of ultrasonic pulses
// and sets ECHO high. ECHO goes low when echo of the burst is received, or on
// timeout (about 38mS).

// In order to provide precise timing, the SR04_TRIG and SR04_ECHO GPIOs must
// be constants, and interrupts may be disabled up to 26 milliseconds. To
// prevent loss of ticks, set TICKMS to 16.

#if !defined(SR04_TRIG) || !defined(SR04_ECHO)
#error "Must define pins SR04_TRIG and SR04_ECHO"
#endif

void init_sr04(void)
{
    OUT_GPIO(SR04_TRIG); // trigger is an output
}


// 8-cycle loops in N microseconds, assumes MHZ is 8 or 16
#define uS(N) (N*(MHZ/8U))

// Trigger the SR04 and return echo pulse width converted to centimeters.
// Return -1 if pulse timed out (> 25 mS) or -2 if SR04 didn't respond. Can
// block with interrupts disabled up to 26 mS.  Do not call more often than
// once per 100mS.
int16_t get_sr04(void)
{
    uint16_t hloops=uS(1000);   // 1 mS
    uint16_t lloops=uS(25000);  // 25 mS

    SET_GPIO(SR04_TRIG);
    waituS(10);
    uint8_t sreg=SREG;
    cli();
    CLR_GPIO(SR04_TRIG);
    // wait 1mS for ECHO to go high
    while (!GET_GPIO(SR04_ECHO)) { if (!--hloops) goto err; __asm__ __volatile__ ("nop\n\tnop"); } // loop is 8 cycles
    // wait 25 mS for ECHO to go low
    while ((GET_GPIO(SR04_ECHO))) { if (!--lloops) goto err; __asm__ __volatile__ ("nop\n\tnop"); } // loop is 8 cycles
    SREG=sreg;
    // Round trip time is ~58 uS per centimeter
    return (uS(25000)-lloops)/uS(58);

  err:
    sei();
    if (!hloops) return -2; // no response from module
    return -1;              // no echo return
}
