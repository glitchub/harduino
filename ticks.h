// "Millisecond" tick counter

// (re)start the tick counter with specified initial value
void start_ticks(uint32_t initial);

// stop the tick counter
void stop_ticks(void); 

// return ticks (aka milliseconds) since start_ticks, or 0 if counter is
// stopped. 
uint32_t get_ticks(void);

