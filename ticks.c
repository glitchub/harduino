// Millisecond tick counter using TIMER0

#include <avr/interrupt.h>

// count millisecond ticks, wraps in about 50 days
static volatile unsigned long ticks;
ISR(TIMER0_COMPA_vect)
{
    ticks++;
}

// stop the tick counter
void stop_ticks(void)
{
    TIMSK0 = 0;     // disable interrupt
    TCCR0B = 0;     // disable timer
    ticks = 0;      // clear count
}

// (re)start the tick counter
void start_ticks(void)
{
    stop_ticks();
    TCNT0 = 0;      // start from 0
    TCCR0A = 2;     // CTC mode
#ifndef F_CPU
#error Must define F_CPU
#elif (F_CPU==16000000UL)
    OCR0A = 249;    // interrupt every 250 clocks
    TCCR0B = 3;     // 1/64 clock == 250Khz
#elif (F_CPU==8000000UL)
    OCR0A = 124;    // interrupt every 125 clocks
    TCCR0B = 3;     // 1/64 clock == 125Kz
#else
#error "F_CPU not supported"
#endif
    TIMSK0 = 2;     // enable OCIE0A interrupt
    sei();
}

// get millseconds since last start_ticks(), or 0 if clock is stopped
unsigned long get_ticks(void)
{
    unsigned char sreg = SREG;
    cli();
    unsigned long t=ticks;
    SREG = sreg;
    return t;
}
