// Alleged Random Number Generator.

// Initialize the random number generator.
void init_arng(void);

// Return an allegedly random 32-bit number. Wait at least 1000mS after
// init_arng() before first use (but the longer the better).
uint32_t get_arng(void);
