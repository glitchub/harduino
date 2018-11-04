// DHT11 temp/humidity sensor driver

// Initialize DHT11
void init_dht11(void);

// Read DHT11 and set dc = degrees C and rh = relative humidity percent and
// return 0, or non-zero on error. Do not call for at least 1 second after
// power on, or more often than every 2 seconds.
int8_t get_dht11(uint8_t *dc, uint8_t *rh);
