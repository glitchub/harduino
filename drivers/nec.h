// Receive NEC IR

// Stop NEC IR receiver 
void stop_nec(void);

// (Re)start NEC IR receiver    
void start_nec(void);

#define NEC_PRESSED 1       // key just pressed
#define NEC_RELEASED 2      // key released (repeat timeout)

// Return 0 if no key event. Otherwise set *key and return PRESSED or
// RELEASED.
int8_t read_nec(uint32_t *key);

// Extract vendor ID, event code or check byte from a NEC key code. In theory
// the check byte should be binary inverse of event byte, but some vendors e.g.
// TiVo use this for remote ID etc.
#define NEC_VENDOR(key) (uint16_t)((key)&0xffff)
#define NEC_EVENT(key) (uint8_t)(((key)>>16)&0xff)
#define NEC_CHECK(key) (uint8_t)(((key)>>24)&0xff)
