// Enable 16mS watchdog interrupt, it is asynchronous to the CPU clock, and
// maybe even aperiodic as a function of temperature, supply voltage, etc.
void init_rng(void);

// Return a random 16-bit number. Wait at least 128 mS after init_rng before
// use.
uint16_t get_rng(void);
