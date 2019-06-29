// Unipolar stepper motor driver, uses TIMER0

#define FMAX (F_CPU/1024)           // max stepper frequency is CPU clock / 1024
#define FMIN (FMAX/256)             // min stepper frequency is MAX/256

#if (STEPPER_SLOW_HZ < FMIN) || (STEPPER_FAST_HZ > FMAX) || (STEPPER_SLOW_HZ > STEPPER_FAST_HZ)
#error "STEPPER_SLOW_HZ or STEPPER_FAST_HZ out of range"
#endif

// timer ticks corresponding to minimum and maximum stepper frequency (round up, to lower frequency)
#define FAST_CLOCKS ((FMAX+STEPPER_FAST_HZ-1)/STEPPER_FAST_HZ)
#define SLOW_CLOCKS ((FMAX+STEPPER_SLOW_HZ-1)/STEPPER_SLOW_HZ)

// steps required to decelerate the stepper from fast to slow
#define STOP_STEPS (SLOW_CLOCKS-FAST_CLOCKS)

// For lack of a better standard terminology we define the coils to be "north",
// "east", south" and "west", and they are sequenced in that order.  These
// terms may or may not correlate to documented numbers 1, 2, 3, and 4; or A1,
// B1, A2, and B2; or some other completely random thing in the motor
// documentation.
#if !defined(STEPPER_N) || !defined(STEPPER_E) || !defined(STEPPER_S) || !defined(STEPPER_W)
#error "Must define pins STEPPER_N, STEPPER_E, STEPPER_S, and STEPPER_W"
#endif

#define NORTH 1
#define EAST 2
#define SOUTH 4
#define WEST 8

#if STEPPER_PHASES==4                   // aka full step
static uint8_t phases[4] = { NORTH|EAST, SOUTH|EAST, SOUTH|WEST, NORTH|WEST };
#elif STEPPER_PHASES==8                 // aka half step
static uint8_t phases[8] = { NORTH, NORTH|EAST, EAST, SOUTH|EAST, SOUTH, SOUTH|WEST, WEST, NORTH|WEST };
#else
#error "Must define STEPPER_PHASES 4 or 8"
#endif

static volatile int8_t forward;             // 1 = step forward, 0 = step backward
static volatile uint16_t steps;             // number of steps to make
static volatile uint8_t clocks;             // clocks per step, the update frequency
static volatile uint8_t phase=0;            // current motor phase

ISR(TIMER0_COMPA_vect)
{
    if (!steps--)
    {
        TIMSK0 = 0;                         // disable interrupt
        TCCR0B = 0;                         // disable timer
        // coils off
        CLR_GPIO(STEPPER_N);
        CLR_GPIO(STEPPER_E);
        CLR_GPIO(STEPPER_S);
        CLR_GPIO(STEPPER_W);
        return;
    }

    // advance to next
    if (forward) phase++; else phase--;

    // energize coils of interest
    uint8_t p = phases[phase % STEPPER_PHASES];
    if (p & NORTH) SET_GPIO(STEPPER_N); else CLR_GPIO(STEPPER_N);
    if (p & EAST) SET_GPIO(STEPPER_E); else CLR_GPIO(STEPPER_E);
    if (p & SOUTH) SET_GPIO(STEPPER_S); else CLR_GPIO(STEPPER_S);
    if (p & WEST) SET_GPIO(STEPPER_W); else  CLR_GPIO(STEPPER_W);

    if (clocks < SLOW_CLOCKS-steps)         // decelerate near the end
    {
        clocks = SLOW_CLOCKS-steps;
        OCR0A = TCNT0+clocks-1;             // schedule next interrupt
    }
    else                                    // else maybe accelerate
    {
        OCR0A = TCNT0+clocks-1;             // schedule next interrupt
        if (clocks > FAST_CLOCKS) clocks--;
    }
}

// Enable stepper driver
void init_stepper(void)
{
    CLR_GPIO(STEPPER_N);    // coils off
    CLR_GPIO(STEPPER_E);
    CLR_GPIO(STEPPER_S);
    CLR_GPIO(STEPPER_W);
    OUT_GPIO(STEPPER_N);       // set as outputs
    OUT_GPIO(STEPPER_E);
    OUT_GPIO(STEPPER_S);
    OUT_GPIO(STEPPER_W);
}

// Given number of steps, (re)start the stepper. Step forward if steps > 0,
// backwards if steps < 0, or stop if steps==0.
//
// The stepper runs in background and stops automatically when specified steps
// have been made.
//
// The maximum step value is +/-32767, keep calling run_stepper() periodicially
// to renew the step count if you don't want the stepper to stop.
//
// If the stepper is currently running and you try to change direction, then
// this function may block up to 16 mS. You can avoid this by stopping the
// stepper, wait for it to stop, then start in the other direction.
//
// This WILL turn interrupts on!!
void run_stepper(int16_t s)
{
    int8_t f=1;                             // forward if positive
    if (s < 0) f=0, s=-s;                   // backward if negative
    cli();
    if (TIMSK0)
    {
        if (!s || f == forward)
        {
            // already going in the right direction
            if (s > STOP_STEPS) steps = s;
            else if (steps > STOP_STEPS) steps = STOP_STEPS;
            // else it's already almost done
            sei();
            return;
        }
        // oops we have to go the other way
        if (steps > STOP_STEPS) steps=STOP_STEPS;
        sei();
#ifndef THREADED
        while (TIMSK0);                     // Spin here until motor stops...
#else
        while (TIMSK0) yield();
#endif
    } else
    {
        sei();
        if (!s) return;
    }
    forward = f;                            // Set direction
    steps = s;                              // Set steps
    clocks = SLOW_CLOCKS;                   // Start at slowest speed
    TCCR0B = 0;                             // Stop timer
    TCNT0 = 0;                              // Count from now
    OCR0A = 1;                              // Interrupt soon
    TIFR0 = 0xff;                           // But not yet
    TCCR0A = 0;                             // Normal mode
    TCCR0B = 5;                             // Run timer at F_CPU/1024
    TIMSK0 = 2;                             // Enable OCIE0A interrupt
}
