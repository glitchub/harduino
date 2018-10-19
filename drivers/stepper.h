// Stepper motor support

// Disable driver and release pins.  
void disable_stepper(void);

// Enable driver in preparation for calls to run_stepper().
void enable_stepper(void);

// Given direction 1=forward or 0=backward, and number of steps, (re)start the
// stepper, or stop it if steps == 0. Returns ASAP, the stepper runs in the
// background. Maximum steps is 65535, you're expected to keep restarting if you
// don't want the stepper to stop.
void run_stepper(uint8_t direction, uint16_t steps);

// Return true if stepper running, false if not.
int8_t check_stepper(void);
