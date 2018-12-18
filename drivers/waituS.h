// Delay up to 16384 microseconds
static inline void waituS(uint16_t N)
{
    if (!N) return;
#if F_CPU == 16000000L
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
#elif F_CPU == 8000000L
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
#error "F_CPU not supported"
#endif
}
