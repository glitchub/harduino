// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

static volatile uint32_t ticks;
#ifdef THREADED
static semaphore tick_sem;                  // released by the ISR to wake the ticker thread
#define resolution 8                        // how many ticks per release, should be a power of 2!
#endif
ISR(TIMER2_COMPA_vect)
{
    ticks++;                                // this will wrap about every 50 days
#ifdef THREADED
    if (!(ticks % resolution))              // wake ticker thread every few
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

// Ticker thread initializes the tick interrupt and then acts as the interrupt
// "bottom half", releasing sleeping threads with expired timers.
THREAD(ticker, 50)
{
    // first init the tick in interrupt
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
    while(1)
    {
        suspend(&tick_sem);                 // suspend until interrupt
        if (!sleeping) continue;
        sleeping->ticks -= resolution;      // decrement first sleeping thread
        while (sleeping->ticks <= 0)        // expired?
        {
            release(&(sleeping->sem));      // release it
            sleeping=sleeping->next;        // advance to next sleeper
            if (!sleeping) break;           // none left?
        }
    }
}

// insert sleeper s into sleeping list
// saves couple bytes of stack this way
inline static void insert(struct sleeper *s)
{
    if (!sleeping)
    {
        sleeping=s;
        return;
    }
        struct sleeper **sp = &sleeping;    // for each sleeping thread
        while (1)
        {
       if ((*sp)->ticks < s->ticks)     // if they expire before us
            {
           s->ticks -= (*sp)->ticks;    // we go after, decrement our count
                if (!(*sp)->next)           // end of list?
                {
               (*sp)->next=s;           // just link us in
                    break;
                }
                sp=*(void **)sp;            // else keep descending
            }
            else
            {
           (*sp)->ticks -= s->ticks;    // they expire after us, adjust their count
           s->next=(*sp);               // and insert us
           *sp=s;
                break;
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
    insert(&s);
    suspend(&s.sem);                       // suspend here until tick thread releases us
}

#else
// Not threaded, call init_ticks() from main to start tick interrupt.
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
}
#include <avr/sleep.h>

// Not threaded, spin for specified ticks, sleeps if sleep_mode() has been set
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

// Return current tick count
uint32_t get_ticks(void)
{
    uint8_t sreg = SREG;
    cli();
    uint32_t t=ticks;
    SREG = sreg;
    return t;
}
