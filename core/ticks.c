// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

static volatile uint32_t ticks;
#ifdef THREAD
static semaphore tick_sem;                  // released by the ISR to wake the ticker thread
#endif
ISR(TIMER2_COMPA_vect)
{
    ticks+=TICKMS;                          // this will wrap about every 50 days
#ifdef THREAD
    release(&tick_sem);
#endif
}

// initialize tick interrupt
static void start(void)
{
    TCNT2 = 0;                              // set initial count
    ticks = 0;
    TCCR2A = 2;                             // CTC mode

    // Set actual milliseconds per tick. This controls how often the CPU wakes
    // up when sleeping, which affects the power consumption.
#if MHZ==8
  #if TICKMS==1
    TCCR2B = 4;                             // 8 Mhz, 1 mS: div=64, count=125
    OCR2A = 124;
  #elif TICKMS==2
    TCCR2B = 5;                             // 8 Mhz, 2 mS: div=128, count=125
    OCR2A = 124;
  #elif TICKMS<=4
    TCCR2B = 6;                             // 8 Mhz, 4 mS: div=256, count=125
    OCR2A = 124;
  #elif TICKMS<=8
    TCCR2B = 6;                             // 8 Mhz, 8 mS: div=256, count=250
    OCR2A = 249;
  #elif TICKMS<=16
    TCCR2B = 7;                             // 8 Mhz, 16 mS: div=1024, count=125
    OCR2A = 124;
  #elif TICKMS<=32
    TCCR2B = 7;                             // 8 Mhz, 32 mS: div=1024, count=250
    OCR2A = 249;
  #else
    #error Max TICKMS is 32 mS
  #endif
#elif MHZ==16
  #if TICKMS==1
    TCCR2B = 5;                             // 16 Mhz, 1 mS: div=128, count=125
    OCR2A = 124;
  #elif TICKMS==2
    TCCR2B = 6;                             // 16 Mhz, 2 mS: div=256, count=125
    OCR2A = 124;
  #elif TICKMS<=4
    TCCR2B = 6;                             // 16 Mhz, 4 mS: div=256, count=250
    OCR2A = 249;
  #elif TICKMS<=8
    TCCR2B = 7;                             // 16 Mhz, 8 mS: div=1024, count=125
    OCR2A = 124;
  #elif TICKMS<=16
    TCCR2B = 7;                             // 16 Mhz, 16 mS: div=1024, count=250
    OCR2A = 249;
  #else
    #error Max TICKMS is 16 mS
  #endif
#else
  #error Unsupported MHZ
#endif

    TIMSK2 = 2;                             // enable OCIE2A interrupt
}

#ifdef THREAD
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
    start();
#if WATCHDOG > 0
    // Enable watchdog, the timeout will be at *least* the specified number of milliseconds.
    // Note the WDTO_XXX symbols are misnamed!
  #if WATCHDOG <= 16
    wdt_enable(WDTO_15MS);
  #elif WATCHDOG <= 32
    wdt_enable(WDTO_30MS);
  #elif WATCHDOG <= 64
    wdt_enable(WDTO_60MS);
  #elif WATCHDOG <= 128
    wdt_enable(WDTO_120MS);
  #elif WATCHDOG <= 256
    wdt_enable(WDTO_250MS);
  #elif WATCHDOG <= 512
    wdt_enable(WDTO_500MS);
  #elif WATCHDOG <= 1024
    wdt_enable(WDTO_1S);
  #elif WATCHDOG <= 2048
    wdt_enable(WDTO_2S);
  #elif WATCHDOG <= 4096
    wdt_enable(WDTO_4MS);
  #elif WATCHDOG <= 8192
    wdt_enable(WDTO_8MS);
  #else
    #error Max WATCHDOG is 8192 mS
  #endif
#endif
    while(1)
    {
#if WATCHDOG > 0
        wdt_reset();                        // reset watchdog
#endif
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
    suspend(&s.sem);                        // suspend here until tick thread releases us
}
#else
// Not threaded, must call init_ticks() from main to start tick interrupt.
void init_ticks(void)
{
    start();
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();
}

// Sleep for specified ticks
void sleep_ticks(int32_t t)
{
    if (t <= 0) return;                     // meh
    cli();
    uint32_t u=ticks+t;
    while ((int32_t)(u-ticks)>0)            // while not expired
    {
        sei();
        sleep_cpu();
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
