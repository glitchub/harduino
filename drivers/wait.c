// waituS and waitmS delay loops

// Delay up to MAXUS microseconds
void waituS(uint16_t N)
{
    // 6 cycles to get here if N is literal constant + 4 cycles for the return
    // == overhead is 10 cycles
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
    // spin n * 4 cycles
    __asm__ __volatile__ (
        "1: sbiw %0,1" "\n\t"          
        "brne 1b" : "=w" (N) : "0" (N) 
    );
}

// Delay up to 65535 milliseconds
void waitmS(uint16_t N)
{
#if F_CPU == 16000000UL
    // MAXUS is 16384
    while (N > 16) waituS(16000-1), N -= 16;  
#elif F_CPU == 8000000UL
    // MAXUS is 32768
    while (N > 32) waituS(32000-2), N -= 32; 
#else
#error "F_CPU not supported"    
#endif
    if (N) waituS(N*1000);
}
