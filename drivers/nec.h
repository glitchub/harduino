// Receive NEC IR

// Init NEC IR receiver
void init_nec(void);

// If IR key pressed, set *key and return 1. If IR key released, set *key and
// return -1.  Otherwise return 0.
// It's possible to get a new key press without a previous key release.
int8_t get_nec(uint32_t *key);

// Extract vendor ID, key code or check byte from a NEC key code. In theory the
// check byte is the binary inverse of the key code, but some vendors e.g. TiVo
// use this for other things.
#define NEC_VENDOR(key) (uint16_t)((key)&0xffff)
#define NEC_KEY(key) (uint8_t)(((key)>>16)&0xff)
#define NEC_CHECK(key) (uint8_t)(((key)>>24)&0xff)
