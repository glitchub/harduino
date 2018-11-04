// Tick counter, using TIMER2. In theory we're counting milliseconds, but
// Arduino resonator is wildly inaccurate so let's just call them 'ticks'
// instead.

// Setup tick interrupt and start counting ticks
void init_ticks(void);

// Return ticks (aka milliseconds) since init_ticks
uint32_t get_ticks(void);

// Sleep for specified ticks. If threading, the calling thread is suspended.
void sleep_ticks(int32_t ticks);

// True it tick value t is less than current ticks, i.e. t is a timer
// that has expired. This works as longer as value of t is within 2^31 ticks
// of current timer.
#define expired(t) ((int32_t)(get_ticks()-(t))>=0)
