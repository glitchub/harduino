// (re)start the counter
void start_ticks(void);

// stop the counter
void stop_ticks(void); 

// return milliseconds since start_ticks, or 0 if counter is stopped. 
uint32_t get_ticks(void);

