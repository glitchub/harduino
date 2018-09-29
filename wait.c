// waituS and waitmS

// Delay up to MAXUS microseconds
void waituS(unsigned int N)
{
    // 6 or 8 cycles to get here, depending on whether n is literal constant or
    // fetched from variable. + 4 cycles for the return, base overhead is 12 cyclesv
#if F_CPU == 16000000UL
    // 16 cycles per microsecond 
	if (N <= 1) return;     // + 3 cycles - took 14-16 cycles just to get here
	N <<= 2;                // + 4 cycles - do 4 loops per uS 
	N -= 5;                 // + 2 cycles == total overhead 19-21 cycles, subtract 5 loops  
#elif F_CPU == 8000000UL
    // 8 cycles per microsecond
	if (N <= 2) return;     // + 3 cycles - took 14-16 cycles just to get here
    N <<= 1;                // + 2 cycles - do 2 loops per uS
	N -= 4;                 // + 2 cycles == total overhead 17-19 cycles, subtract 4 loops
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
void waitmS(unsigned int N)
{
#if F_CPU == 16000000UL
    while (N > 16) waituS(16000-1), N -= 16;  
#elif F_CPU == 8000000UL
	while (N > 32) waituS(32000-2), N -= 32; 
#else
#error "Invalid F_CPU"    
#endif
    if (N) waituS(N*1000);
}
