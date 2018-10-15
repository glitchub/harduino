// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

// Accrue ticks, the counter will wrap about every 50 days.
static volatile uint32_t ticks;
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
void start_ticks(uint32_t initial)
{
    stop_ticks();
    TCNT2 = 0;          // start from initial value
    ticks = initial;
    TCCR2A = 2;         // CTC mode
#if F_CPU==16000000L
    OCR2A = 249;        // interrupt every 250 clocks
    TCCR2B = 4;         // 1/64 clock == 250Khz
#elif F_CPU==8000000L
    OCR2A = 124;        // interrupt every 125 clocks
    TCCR2B = 4;         // 1/64 clock == 125Kz
#else
#error "F_CPU not supported"
#endif
    TIMSK2 = 2;         // enable OCIE0A interrupt
}

// Return tick count since last start_ticks() (or 0 if ticks are stopped).
uint32_t read_ticks(void)
{
    uint8_t sreg = SREG;
    cli();
    uint32_t t=ticks;
    SREG = sreg;
    return t;
}