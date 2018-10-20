// Stepper motor support

// Disable driver and release pins.
void disable_stepper(void);

// Enable driver in preparation for calls to run_stepper().
void enable_stepper(void);

// Given number of steps, (re)start the stepper. Step forward if steps > 0,
// backwards if steps < 0, or stop if steps==0. If changing direction, may
// block up to 16mS. Otherwise the stepper runs in background and stops
// automatically when specified steps have been made. The maximum value for
// steps is +/-32767, keep calling this periodicially if you don't want the
// stepper to stop.
// This function WILL turn interrupts on!!
void run_stepper(int16_t steps);

// True if stepper is running, false if not.
#define check_stepper() (TIMSK0!=0)
