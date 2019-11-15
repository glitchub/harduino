// Alleged Random Number Generator

// This can only be used if the CPU uses an external crystal/resonator, and the
// system watchdog is disabled (WATCHDOG=0). The watchdog is driven by an
// internal RC which is asynchronous to the CPU clock, and can therefore be
// used as a source of entropy.

#if WATCHDOG > 0
#error Must define WATCHDOG=0 to use arng
#endif

// lfsr polynomials selected at random from
// https://users.ece.cmu.edu/~koopman/lfsrz
#define poly32 0xB4BCD35C // 32-bit
#define poly31 0x7A5BC2E3 // 31-bit

// We maintain two LSFRs and XOR them together to produce the output.
static uint32_t lsfr31 = poly31;
static uint32_t lsfr32 = poly32;

static inline uint32_t lfsr(uint32_t l, uint32_t p)
{
    return l>>1 ^ ((l&1) ? p : 0);
}

// Watchdog interrupt, maybe spin lsfr31
ISR(WDT_vect)
{
    if (TCNT2 & 1) lsfr31 = lfsr(lsfr31, poly31);
}

// Initialize the watchdog interrupt
void init_arng(void)
{
    WDTCSR = 0x10;      // allow watchdog change
    WDTCSR = 0x40;      // interrupt every 16mS
}

// Spin lsfr32 and xor with lxfr31 to produce a 'random' number
uint32_t get_arng(void)
{
    lsfr32 = lfsr(lsfr32, poly32);
    uint8_t sreg=SREG;
    cli();
    uint32_t r = lsfr31 ^ lsfr32;
    SREG=sreg;
    return r;
}
