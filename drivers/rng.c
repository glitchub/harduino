// lfsr polynomials selected at random from https://users.ece.cmu.edu/~koopman/lfsr, e.g.:
// curl https://users.ece.cmu.edu/~koopman/lfsr/32.dat.gz | gunzip | awk 'BEGIN{srand()} NR*rand()<1{s=$0} END{print s}'
#define p1 0xB4BCD35C // 32-bit
#define p2 0x7A5BC2E3 // 31-bit

static uint32_t l1=p1, l2=p2;

static inline uint32_t lfsr(uint32_t l, uint32_t p)
{
    return (l >> 1) ^ ((l & 1) ? p : 0);
}

// Random number generation, requires ticks() to be running! This uses the
// watchdog interrupt which is presumwed asynchronous to the CPU clock and
// therefore the value of TCNT2 can be used as entropy.

// On watchdog interrupt, collect entropy in lfsr1
uint16_t entropy;
ISR(WDT_vect)
{
    l2 = lfsr(l2, p2^TCNT2);
}

// Enable watchdog interrupt
void init_rng(void)
{
    WDTCSR = 0x18;  // allow watchdog changes
    WDTCSR = 0x40;  // interrupt every 16mS
}

// Return an allegedly random 16-bit number. Wait at least 1000mS after
// init_rng before first use (but the longer the better).
uint16_t get_rng(void)
{
    l1=lfsr(l1,p1);
    l1=lfsr(l1,p1);
    char sreg=SREG;
    cli();
    l2=lfsr(l2,p2);
    uint16_t r=l1^l2;
    SREG=sreg;
    return r;
}
