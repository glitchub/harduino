// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

static volatile uint32_t ticks;             // Accrue ticks, the counter will wrap about every 50 days.
#ifdef THREADED
static semaphore tick_sem;                  // Also wake ticker thread
#endif
ISR(TIMER2_COMPA_vect)
{
    ticks++;
#ifdef THREADED
    release(&tick_sem);
#endif
}

#ifdef THREADED
// context for sleeping thread
volatile struct sleeper
{
    struct sleeper *next;                   // Link to next sleeper, this must be first
    int32_t ticks;                          // How many ticks remain
    semaphore sem;                          // What sleeping thread is suspended on
} sleeper;

// linked list of sleeper structs
static struct sleeper *sleeping;

// This is the tick interrupt "bottom half", release sleeping threads with
// expired timers.
uint8_t ticker_stack[64];
static void ticker(void)
{
    while(1)
    {
        suspend(&tick_sem);                 // suspend until interrupt
        if (!sleeping) continue;
        sleeping->ticks--;                  // decrement first sleeping thread
        while (sleeping->ticks <= 0)        // expired?
        {
            release(&(sleeping->sem));      // release it
            sleeping=sleeping->next;        // advance to next sleeper
            if (!sleeping) break;           // none left?
        }
    }
}

// Suspend calling thread for specified number of ticks. The sleeping list is
// sorted in order of next thread to expire.
void sleep_ticks(int32_t t)
{
    if (t <= 0) return;                     // meh

    struct sleeper s;                       // sleeper struct goes on the stack
    memset(&s, 0, sizeof s);
    s.ticks=t;

    if (!sleeping)                          // nobody sleeping?
    {
        sleeping = &s;                      // now there is
    } else
    {
        struct sleeper **sp = &sleeping;    // for each sleeping thread
        while (1)
        {
            if ((*sp)->ticks < s.ticks)     // if they expire before us
            {
                s.ticks -= (*sp)->ticks;    // we go after, decrement our count
                if (!(*sp)->next)           // end of list?
                {
                    (*sp)->next=&s;         // just link us in
                    break;
                }
                sp=*(void **)sp;            // else keep descending
            }
            else
            {
                (*sp)->ticks -= s.ticks;    // they expire after us, adjust their count
                s.next=(*sp);               // and insert us
                *sp=&s;
                break;
            }
        }
    }
    suspend(&s.sem);                       // suspend here until tick thread releases us
}
#else
// Not threaded, spin for specified ticks, sleeps if sleep_mode() has been set
#include <avr/sleep.h>
void sleep_ticks(int32_t t)
{
    cli();
    uint32_t u=ticks+t;
    while ((int32_t)(u-ticks)>0)            // while not expired
    {
        sei();
        sleep_cpu();                        // no-op if sleep not enabled
        cli();
    }
    sei();
}
#endif

// Enable the tick interrupt
void init_ticks(void)
{
    TCNT2 = 0;          // start from initial value
    ticks = 0;
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
    TIMSK2 = 2;         // enable OCIE2A interrupt
#ifdef  THREADED
    init_thread(ticker, ticker_stack, sizeof ticker_stack);
#endif
}

// Return running tick count
uint32_t get_ticks(void)
{
    uint8_t sreg = SREG;
    cli();
    uint32_t t=ticks;
    SREG = sreg;
    return t;
}
