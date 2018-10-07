// (re)start the tick counter with specified initial value
void start_ticks(unsigned long initial);

// stop the tick counter
void stop_ticks(void); 

// return ticks (aka milliseconds) since start_ticks, or 0 if counter is
// stopped. 
uint32_t get_ticks(void);

