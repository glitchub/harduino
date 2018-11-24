// Receive NEC IR via timer 1 input capture.

// Detector attaches to ICP1. We assume the detector demodulates the IR signal
// (e.g. AZ-1838HS optimized for 38Khz).

// NEC_IDLE defines whether the detector output is high or low when not
// receiving a pulse.
#ifndef NEC_IDLE
#error "Must define NEC_IDLE 0 or 1"
#endif

// NEC IR transmission consist of a preamble, 32 data bits, and a postamble.
//      Preamble: 9000 uS mark and 4500 uS space
//      Zero bit: 560 uS mark and 560 uS space
//      One bit: 560 uS mark and 1690 uS space
//      Postamble: 560 uS mark
// After the initial message is sent, as long as the button is still held a
// repeat code will be sent every 108mS, consisting of:
//      Preamble: 9000 uS mark and 2250 uS space
//      Postamle: 560 uS mark

// We run the clock at F_CPU/8 ticks per second. This converts microseconds to clock ticks.
#if F_CPU==16000000L
#define uS(n) ((n)*2)
#elif F_CPU==8000000L
#define uS(n) (n)
#else
#error "F_CPU not supported"
#endif

// remember last key code, current detector state
static volatile uint32_t keycode, state;

// Reset IR state machine
static inline void reset(void)
{
    TIMSK1 = 0;                         // disable timer interrupts
    state = 0;                          // reset state
#if NEC_IDLE
    TCCR1B &= (uint8_t)~(1 << ICES1);   // detector output normally high, so interrupt on low
#else
    TCCR1B |= 1 << ICES1;               // detector output normally low, so interrupt on high
#endif
    TIFR1 = 0xff;                       // clear pending interrupts
    TIMSK1 = 1 << ICIE1;                // enable input capture
}

#define REPEAT 0xC0DED00D

ISR(TIMER1_COMPA_vect)
{
    // Timeout waiting for next edge. A timeout in state 4 means we received a
    // repeat code, report with magic code.
    if (state == 4) keycode = REPEAT;
    reset();
}

ISR(TIMER1_CAPT_vect)
{
    static uint32_t bits;                               // accrued bits
    static uint16_t edge;                               // time of last edge
    uint16_t width=ICR1-edge;                           // width of last pulse
    edge=ICR1;                                          // remember new edge

    switch (state)
    {
        case 0:                                         // start of preamble mark
            OCR1A = edge + uS(9000+900);                // expect it to be 9mS (all times are plus or minus 10%)
            TIFR1 |= 1 << OCIE1A;                       // clear any pending timeout interrupt
            TIMSK1 |= 1 << OCF1A;                       // enable timeout interrupt
            break;

        case 1:                                         // end of preamble mark
            if (width < uS(9000-900)) goto irx;         // too short?
            OCR1A = edge + uS(4500+450);                // expect space to be 2.25 mS to 4.5mS
            break;

        case 2:                                         // end of preamble space
            if (width < uS(2250-225)) goto irx;         // too short?
            OCR1A = ICR1 + uS(560+56);                  // expect mark to be 560uS
            break;

        default:                                        // data bits
            if (width < uS(560-56)) goto irx;           // too short?
            if (state & 1)                              // if odd: end of data mark
                OCR1A = edge + uS(1690+169);            // expect space to be up to 1690 uS
            else                                        // if even: end of data space
            {
                OCR1A = edge + uS(560+56);              // expect mark to be 560uS
                bits >>= 1;                             // note another bit
                if (width>1125) bits |= 0x80000000;     // long space is a 1
            }
            break;

        case 67:                                        // end of last pulse
            if (width < uS(560-56)) goto irx;           // too short?
            keycode = bits;                             // ok remember new code
          irx:
            reset();                                    // reset state machine
            return;

    }
    TCCR1B ^= (1 << ICES1);                             // toggle edge of interest
    state++;                                            // advance to next state
}

static uint32_t pressed;

// Init NEC IR receiver
void init_nec(void)
{
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 2; // clock/8
    reset();
}

// Return 0 if no key event, else set *key and return PRESS for new key or
// RELEASE for new release (timeout). Note it is possible to get a new PRESS
// without a prior RELEASE.
// Assumes start_ticks() has been called.
int8_t get_nec(uint32_t *key)
{
    static uint32_t timeout;
    uint8_t sreg = SREG;
    cli();
    uint32_t k = keycode;
    keycode = 0;
    SREG = sreg;

    switch(k)
    {
        case 0:
            if (!pressed || !expired(timeout)) return 0;
            *key=pressed;
            pressed=0;
            return NEC_RELEASED;

        case REPEAT:
            if (pressed) timeout=get_ticks()+110;
            return 0;

        default:
            *key = pressed = k;
            timeout=get_ticks()+110;
            return NEC_PRESSED;
    }
}
