// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.
static volatile uint32_t ticks;
static semaphore tick_sem;                  // released by the ISR to wake the ticker thread
ISR(TIMER2_COMPA_vect)
{
    ticks+=TICKMS;                          // this will wrap about every 50 days
        release(&tick_sem);
}

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
    TCNT2 = 0;                              // start from initial count
    ticks = 0;
    TCCR2A = 2;         // CTC mode

#if MHZ==8
    #if TICKMS==1
        // 8 Mhz, 1 mS: div=64, count=125
        TCCR2B = 4;
        OCR2A = 124;
    #elif TICKMS==2
        // 8 Mhz, 2 mS: div=128, count=125
        TCCR2B = 5;
        OCR2A = 124;
    #elif TICKMS==4
        // 8 Mhz, 4 mS: div=256, count=125
        TCCR2B = 6;
        OCR2A = 124;
    #elif TICKMS==8
        // 8 Mhz, 8 mS: div=256, count=250
        TCCR2B = 6;
        OCR2A = 249;
    #elif TICKMS==16
        // 8 Mhz, 16 mS: div=1024, count=125
        TCCR2B = 7;
        OCR2A = 124;
    #elif TICKMS==32
        // 8 Mhz, 32 mS: div=1024, count=250
        TCCR2B = 7;
        OCR2A = 249;
    #else
        #error "Unsupported TICKMS"
    #endif
#elif MHZ==16
    #if TICKMS==1
        // 16 Mhz, 1 mS: div=128, count=125
        TCCR2B = 5;
        OCR2A = 124;
    #elif TICKMS==2
        // 16 Mhz, 2 mS: div=256, count=125
        TCCR2B = 6;
        OCR2A = 124;
    #elif TICKMS==4
        // 16 Mhz, 4 mS: div=256, count=250
        TCCR2B = 6;
        OCR2A = 249;
    #elif TICKMS==8
        // 16 Mhz, 8 mS: div=1024, count=125
        TCCR2B = 7;
        OCR2A = 124;
    #elif TICKMS==16
        // 16 Mhz, 16 mS: div=1024, count=250
        TCCR2B = 7;
        OCR2A = 249;
    #else
        #error "Unsupported TICKMS"
    #endif
#else
    #error "Unsupported MHZ"
#endif

    TIMSK2 = 2;         // enable OCIE2A interrupt
    wdt_enable(WDTO_120MS);                 // start watchdog
    while(1)
    {
        wdt_reset();                        // reset watchdog
        suspend(&tick_sem);                 // suspend until interrupt
        if (!sleeping) continue;
        sleeping->ticks -= TICKMS;
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

// Return current tick count
uint32_t get_ticks(void)
{
    uint8_t sreg = SREG;
    cli();
    uint32_t t=ticks;
    SREG = sreg;
    return t;
}
