// Support for SR04 ultrasonic ranging module, requires an output pin for
// trigger and an input pin for echo.

// Initialize the SR04 pins
void init_sr04(void);

// Trigger SR04 measurement cycle, return distance in cm, or -1 if timeout, or
// -2 if device did not respond. This function may block up to 25 milliseconds,
// and should not be called more often than once per 100 mS.
int16_t get_sr04(void);
