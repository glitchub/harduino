// SR04 ultrasonic ranging driver

// 8-cycle loops in N microseconds, for MHZ == 8 or 16
#define uS(N) (N*(MHZ/8U))

// The SR04 module has TRIG input and ECHO output, normally both low. When the
// module sees TRIG go high for 10uS it transmits a burst of ultrasonic pulses
// and sets ECHO high. ECHO goes low when echo of the burst is received, or on
// timeout.

// get_sr04() can spin with interrupts disabled for up to 26 mS
#if !defined(TICKMS) || TICKMS < 16
#warning TICKMS should be at least 16 to avoid losing ticks
#endif

// Given trigger and echo gpios, trigger the SR04 and return the echo pulse
// width converted to centimeters. Return -2 if SR04 didn't respond, or -1 if
// no echo was received within 25 mS.
// Do not call more often than once per 100 mS.
int16_t get_sr04(gpio *trigger, gpio *echo)
{
    out_gpio(trigger);
    in_gpio(echo);

    clr_gpio(trigger);
    waituS(10);
    set_gpio(trigger);
    waituS(10);

    uint8_t sreg=SREG;
    cli();
    clr_gpio(trigger);

    // wait 1mS for echo to go high
    uint16_t loops=uS(1000);
    asm volatile (
        // loop is 8 cycles
        "; loops=%0 (%a0) bit=%2 *pin=%3 (%a3)\n"
        "1: sbiw %a0,  \n" // 2, decrement loops
        "   breq 2f    \n" // 1, exit if zero
        "   ld r0, %a3 \n" // 2, get *pin
        "   and r0, %2 \n" // 1, test bit
        "   breq 1b    \n" // 2, repeat if clear
        "2:            \n"
        : "=&e" (loops)
        : "0" (loops), "r" (echo->bit), "e" (echo->pin)
    );
    if (!loops)
    {
        SREG = sreg;
        return -2;
    }
    // wait 25 mS for echo to go low, loop is 8 cycles
    loops=uS(25000);
    asm volatile (
        // loop is 8 cycles
        "; loops=%0 (%a0) bit=%2 *pin=%3 (%a3)\n"
        "1: sbiw %a0, 1\n" // 2, decrement loops
        "   breq 2f    \n" // 1, exit if zero
        "   ld r0, %a3 \n" // 2, get *pin
        "   and r0, %2 \n" // 1, test bit
        "   brne 1b    \n" // 2, repeat if set
        "2:            \n"
        : "=&e" (loops)
        : "0" (loops), "r" (echo->bit), "e" (echo->pin)
    );
    SREG = sreg;
    if (!loops) return -1;

    // Round trip time is ~58 uS per centimeter
    return (uS(25000)-loops)/uS(58);
}
