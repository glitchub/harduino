// Stepper motor support

// You must define GPIOs named STEPPER_N, STEPPER_E, STEPPER_S, and STEPPER_W,
// which control the stepper's "north", "east", south" and "west" coils (for
// lack of a better term... may or may not correlate to documented numbers 1,
// 2, 3, and 4; or A1, B1, A2, and B2; or some other completely random thing in
// the motor documentation).

// By default the driver uses half-stepping aka 8 phases per rotation, with
// coils energized in order: N NE E SE S SW W NW (or the reverse).

// If STEPPER_FULL is defined the the driver will use full-step aka 4 steps per
// rotation, with coils energized in order: NE SE SW NW (or the reverse).

// You can optionally define STEPPER_SLOW_HZ and STEPPER_FAST_HZ to define the
// range of stepper frequencies. The stepper must be able to start at the slow
// frequency, and must not skip at the fast frequency. Default is 500 and 1500
// respectively.

// Init driver in preparation for calls to run_stepper().
void init_stepper(void);

// Given number of steps, (re)start the stepper. Step forward if steps > 0,
// backwards if steps < 0, or stop if steps==0. If changing direction, may
// block up to 16mS. Otherwise returns immediately. The stepper runs in
// background and stops automatically when specified steps have been made. The
// maximum value for steps is +/-32767, so the maximum run time is at least
// 32767/STEPPER_FAST_HZ seconds. To prevent the motor from stopping just keep
// calling run_stepper() to restart the count.
void start_stepper(int16_t steps);

// start the stepper and return when it has stopped
void run_stepper(int16_t steps);

// Return true if stepper is currently running
#define running_stepper() (TIMSK0!=0)
