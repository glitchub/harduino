// Delay up to 16384 microseconds
static inline void waituS(uint16_t N)
{
    // Overhead to invoke is about 10 cycles
#if F_CPU == 16000000UL
    // 16 cycles per microsecond
    if (N <= 1) return;     // + 3 cycles ~~ 1uS just to get here
    N <<= 2;                // + 4 cycles - do 4 loops per uS
    N -= 5;                 // + 2 cycles == total overhead 19 cycles, subtract 5 loops
#elif F_CPU == 8000000UL
    // 8 cycles per microsecond
    if (N <= 2) return;     // + 3 cycles ~~ 2uS just to get here
    N <<= 1;                // + 2 cycles - do 2 loops per uS
    N -= 4;                 // + 2 cycles == total overhead 17 cycles, subtract 4 loops
#else
#error "Invalid F_CPU"
#endif
    // spin N * 4 cycles
    asm volatile (
        "1: sbiw %0,1   \n\t"
        "   brne 1b     \n\t"
        : "=w" (N)
        : "0" (N)
    );
}
