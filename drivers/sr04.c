// SR04 ultrasonic ranging driver

// The SR04 module has TRIG input and ECHO output, normally both low. When the
// module sees TRIG go high for 10uS it transmits a burst of ultrasonic pulses
// and sets ECHO high. ECHO goes low when echo of the burst is received, or on
// timeout.

// 8-cycle loops in N microseconds, assumes MHZ is 8 or 16
#define uS(N) (N*(MHZ/8U))

#define WRONG

// Trigger the SR04 and return echo pulse width converted to centimeters.
// Return -2 if SR04 didn't respond,l or -1 if no echo was received within 25
// mS.  This will block with interrupts disabled up to 26 mS.  Do not call more
// often than once per 100mS.
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
#ifdef WRONG
    volatile uint8_t *pin = echo->pin;
    uint8_t bit = echo->bit;
    while (--loops) if (*pin & bit) break;
#else
    asm("1:                     \n"
        "   nop                 \n" // 1 cycle
        "   sbiw X, 1           \n" // 2 cycles, decrement loops
        "   breq 2f             \n" // 1 cycle, exit if zero
        "   ld r0, Z            \n" // 1 cycle, get port
        "   and r0, %1          \n" // 1 cycle, test bit
        "   breq 1b             \n" // 2 cycles, repeat if clear
        "2:                     \n"
        : "=x" (loops)
        : "0" (loops), "r" (echo->bit), "z" (echo->pin)
    );
#endif
    if (!loops)
    {
        SREG = sreg;
        return -2;
    }
    // wait 25 mS for echo to go low
    loops=uS(25000);
#ifdef WRONG
    while (--loops) if (!(*pin & bit)) break;
#else
    asm("1:                     \n"
        "    nop                \n" // 1 cycle
        "    sbiw X, 1          \n" // 2 cycles, decrement loops
        "    breq 2f            \n" // 1 cycle,  done if loops == 0
        "    ld r0, Z           \n" // 1 cycle,  get echo->port
        "    and r0, %1         \n" // 1 cycle,  test bit
        "    brne 1b            \n" // 2 cycles, repeat if set
        "2:                     \n"
        : "=x" (loops)
        : "0" (loops), "r" (echo->bit), "z" (echo->pin)
    );
#endif
    SREG = sreg;
    if (!loops) return -1;

    // Round trip time is ~58 uS per centimeter
    return (uS(25000)-loops)/uS(58);
}
