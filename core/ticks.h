// Tick counter, using TIMER2. In theory ticks are milliseconds, in practice
// the resonators are inaccurate.

// Initialize tick interrupt
void init_ticks(void);

// Return ticks (aka milliseconds) since boot
uint32_t get_ticks(void);

// Suspend calling thread for specified ticks.
void sleep_ticks(int32_t ticks);

// Sleep until specified tick count is reached (return immediately if diff is
// >= 2^31).
#define sleep_until(t) sleep_ticks((t)-get_ticks())

// True if tick value t is less than current ticks
#define expired(t) ((int32_t)(get_ticks()-(t))>=0)

// Inline delay up to 16384 microseconds
static inline void waituS(uint16_t N)
{
    if (!N) return;
#if MHZ == 16
    // spin N * 16 cycles
    asm volatile (
        "1: nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   sbiw %0,1   \n\t"
        "   brne 1b     \n\t"
        : "=w" (N)
        : "0" (N)
    );
#elif MHZ == 8
    // spin N * 8 cycles
    asm volatile (
        "1: nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   nop         \n\t"
        "   sbiw %0,1   \n\t"
        "   brne 1b     \n\t"
        : "=w" (N)
        : "0" (N)
    );
#else
#error "MHZ not supported"
#endif
}
