// Support for DHT11 temp/humidity sensor.

// Arduino pin to use the DHT11. There must be a 5K to 10K hardware pull-up on this pin.
#define DHT_IO GPIO12 

// Initialize
void start_dht11(void);

// Read DHT11 and set dc = degrees C and rh = relative humidity percent and
// return 0, or non-zero on error. Should not be called until at least 1 second
// after power on, or more often than every 2 seconds.
int8_t read_dht11(uint8_t *dc, uint8_t *rh);
