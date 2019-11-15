// Receive NEC IR via timer 1 input capture.

// Detector attaches to ICP1 (aka GPIO08 on UnoR3, for example). We assume the
// detector demodulates the IR signal (e.g. AZ-1838HS optimized for 38Khz).

// NEC_IDLE defines whether the detector output is high or low when not
// receiving a pulse.
#ifndef NEC_IDLE
// default, assume hight
#define NEC_IDLE 1
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

// Converts microseconds to TIMER1 ticks
#if MHZ==16
#define uS(n) ((n)*2)
#elif MHZ==8
#define uS(n) (n)
#else
#error "MHZ not supported"
#endif

// key queue
#define KEYS 8
static volatile uint32_t keys[KEYS];
static volatile uint8_t head=0, count=0;

// push a key into queue (in interrupt context)
static inline void push(uint32_t key)
{
    if (count < KEYS)
    {
        keys[(head+count)%KEYS]=key;
        count++;
    }
}

// IR detector state
static volatile uint32_t state;

// Reset IR state machine
static inline void reset(void)
{
    TIMSK1 = 0;                                         // disable timer interrupts
    state = 0;                                          // reset state
#if NEC_IDLE
    TCCR1B &= (uint8_t)~(1 << ICES1);                   // detector output normally high, so interrupt on low
#else
    TCCR1B |= 1 << ICES1;                               // detector output normally low, so interrupt on high
#endif
    TIFR1 = 0xff;                                       // clear pending interrupts
    TIMSK1 = 1 << ICIE1;                                // enable input capture
}

#define REPEAT 0xC0DED00D

// interrupt on edge timeout
ISR(TIMER1_COMPA_vect)
{
    // Timeout waiting for next edge. A timeout in state 4 means we received a
    // repeat code, push as code 0.
    if (state == 4) push(0);
    reset();
}

// interrupt on input edge
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
            push(bits);                                 // ok push new key
          irx:
            reset();                                    // reset state machine
            return;

    }
    TCCR1B ^= (1 << ICES1);                             // toggle edge of interest
    state++;                                            // advance to next state
}

// initialize NEC receiver
void init_nec(void)
{
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 2; // clock/8
    reset();
}

// If IR key pressed, set *key and return 1.
// If IR key released, set *key and return -1.
// Otherwise return 0.
// It's possible to get a new key press without a previous key release.
static uint32_t pressed; // last pressed key
static uint32_t timeout; // ticks at last event
int8_t get_nec(uint32_t *key)
{
    if (count)
    {
        // there's something in the queue
        uint8_t sreg = SREG;
        cli();
        *key=keys[head++];
        head %= KEYS;
        count--;
        SREG = sreg;
        timeout=get_ticks()+110;    // restart timer
        if (!*key) return 0;        // no event for repeats
        pressed=*key;
        return 1;                   // key press
    }

    if (pressed && expired(timeout))
    {
        // timeout
        *key=pressed;
        pressed = 0;
        return -1;                  // key release
    }

    return 0;
}
