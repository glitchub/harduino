// Receive NEC IR via timer 1 input capture.

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ticks.h"
#include "nec.h"

// Detector must attach to ICP1, which on Uno is PB0 aka pin 8.

// The detector must demodulate the IR signal (e.g. AZ-1838HS optimized for
// 38Khz). 
//
// Set detector 'idle' output 1=high or 0=low. The AZ-1838HS output is normally
// high, goes low when mark is detected.
#define IDLE 1

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
//#define uS(n) ((F_CPU*(n))/8000000UL)
#define uS(n) ((n)*2)

// remember last key code, current detector state 
static volatile unsigned long keycode, state;

// Reset IR state machine
static inline void reset(void)
{
    TIMSK1 = 0;                 // disable timer interrupts
    state = 0;                  // reset state
#if IDLE == 0
    TCCR1B |= 1 << ICES1;       // detector output normally low, so interrupt on high
#else
    TCCR1B &= ~(1 << ICES1);    // detector output normally high, so interrupt on low
#endif
    TIFR1 = 0xff;               // clear pending interrupts
    TIMSK1 = 1 << ICIE1;        // enable input capture
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
    static unsigned long bits;                          // accrued bits
    static unsigned int edge;                           // time of last edge 
    unsigned int width=ICR1-edge;                       // width of last pulse 
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
    
static unsigned long pressed;

void stop_nec(void)
{
    TIMSK1 = 0;                                         // disable interrupts
    TCCR1B = 0;                                         // disable counter    
    TIFR1 = 0xff;                                       // clear any pending
    keycode = pressed = 0;
}    

// (Re)start NEC IR receiver    
void start_nec(void)
{
    stop_nec();
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 2; // clock/8
    reset();
    sei();      // enable interrupt
}

// Return 0 if no key mevent, else set *key and return PRESS for new key or
// RELEASE for new release (timeout). Note it is possible to get a new PRESS
// without a prior RELEASE.
// Assumes start_ticks() has been called.
int get_nec(unsigned long *key)
{
    static unsigned long since=0;
    unsigned sreg = SREG;
    cli();
    unsigned long k = keycode;
    keycode = 0;
    SREG = sreg;

    switch(k)
    {
        case 0:
            if (!pressed || get_ticks()-since < 110) return 0;
            *key=pressed; 
            pressed=0;
            return RELEASED;
        
        case REPEAT:
            if (pressed) since=get_ticks();
            return 0;

        default:
            *key = pressed = k;
            since = get_ticks();
            return PRESSED;
    }        
}    
