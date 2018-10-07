// Receive NEC IR

// Stop NEC IR receiver
void stop_nec(void);

// (Re)start NEC IR receiver    
void start_nec(void);

#define PRESSED 1       // key just pressed
#define RELEASED 2      // key released (repeat timeout)

// Return 0 if no key event. Otherwise set *key and return PRESSED or
// RELEASED.
int get_nec(unsigned long *key);
