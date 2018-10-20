// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

// (Re)start the tick counter with specified initial value
void enable_ticks(uint32_t initial);

// Stop the tick counter
void disable_ticks(void);

// Return ticks (aka milliseconds) since start_ticks, or 0 if counter is
// stopped.
uint32_t get_ticks(void);

// Sleep for specified ticks
void sleep_ticks(int32_t ticks);

// True it tick value t is less than current ticks, i.e. t is a timer
// that has expired. This works as longer as value of t is within 2^31 ticks
// of current timer.
#define expired(t) ((int32_t)(get_ticks()-(t))>=0)
