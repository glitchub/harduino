// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

#include <avr/interrupt.h>

// Accrue ticks, the counter will wrap about every 50 days.
static volatile unsigned long ticks;
ISR(TIMER2_COMPA_vect)
{
    ticks++;
}

// Stop the tick counter
void stop_ticks(void)
{
    TIMSK2 = 0;         // disable interrupt
    TCCR2B = 0;         // disable timer
}

// (Re)start the tick counter with specified initial value
void start_ticks(unsigned long initial)
{
    stop_ticks();
    TCNT2 = 0;          // start from initial value
    ticks = initial;
    TCCR2A = 2;         // CTC mode
#if (F_CPU==16000000UL)
    OCR2A = 249;        // interrupt every 250 clocks
    TCCR2B = 4;         // 1/64 clock == 250Khz
#elif (F_CPU==8000000UL)
    OCR2A = 124;        // interrupt every 125 clocks
    TCCR2B = 4;         // 1/64 clock == 125Kz
#else
#error "F_CPU not supported"
#endif
    TIMSK2 = 2;         // enable OCIE0A interrupt
    sei();
}

// Get tick count since last start_ticks() (or 0 if ticks are stopped).
unsigned long get_ticks(void)
{
    unsigned char sreg = SREG;
    cli();
    unsigned long t=ticks;
    SREG = sreg;
    return t;
}
