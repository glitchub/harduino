// lfsr polynomials selected at random from
// https://users.ece.cmu.edu/~koopman/lfsrz

#define poly32 0xB4BCD35C // 32-bit
#define poly31 0x7A5BC2E3 // 31-bit

static inline uint32_t lfsr(uint32_t l, uint32_t p)
{
    return l>>1 ^ ((l&1) ? p : 0);
}

// Random number generation, requires ticks() to be running! This uses the
// watchdog interrupt which is presumwed asynchronous to the CPU clock and
// therefore the value of TCNT2 can be used as entropy.
static uint32_t entropy=poly31; // 31-bit LSFR
ISR(WDT_vect)
{
    if (TCNT2 & 1) entropy = lfsr(entropy, poly31);
}

// Enable watchdog interrupt
void init_rng(void)
{
    WDTCSR = 0x18;                  // allow watchdog changes
    WDTCSR = 0x40;                  // interrupt every 16mS
}

// Return an allegedly random 16-bit number. Wait at least 1000mS after
// init_rng before first use (but the longer the better).
static uint32_t rng=poly32;
uint32_t get_rng(void)
{
    rng=lfsr(rng, poly32);      // sequence rng
    uint8_t sreg=SREG;
    cli();
    uint32_t r = rng ^ entropy; // add entropy
    SREG=sreg;
    return r;
}

uint32_t get_entropy(void)
{
    uint8_t sreg=SREG;
    cli();
    uint32_t r = entropy;
    SREG=sreg;
    return r;
}

