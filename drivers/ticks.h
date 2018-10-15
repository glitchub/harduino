// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

// (Re)start the tick counter with specified initial value
void start_ticks(uint32_t initial);

// Stop the tick counter
void stop_ticks(void); 

// Return ticks (aka milliseconds) since start_ticks, or 0 if counter is
// stopped. 
uint32_t read_ticks(void);

