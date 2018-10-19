// Unipolar stepper motor driver, uses TIMER0

#define FMAX (F_CPU/1024)           // max stepper frequency is CPU clock / 1024
#define FMIN (FMAX/256)             // min stepper frequency is MAX/256

#if (STEPPER_SLOW_HZ < FMIN) || (STEPPER_FAST_HZ > FMAX) || (STEPPER_SLOW_HZ > STEPPER_FAST_HZ)
#error "STEPPER_SLOW_HZ or STEPPER_FAST_HZ out of range"
#endif

// timer ticks corresponding to minimum and maximum stepper frequency
#define FAST_CLOCKS ((FMAX+STEPPER_FAST_HZ-1)/STEPPER_FAST_HZ)
#define SLOW_CLOCKS ((FMAX+STEPPER_SLOW_HZ-1)/STEPPER_SLOW_HZ)
#if FAST_CLOCKS < 2
#error "FAST_CLOCKS out of range"
#endif
#if SLOW_CLOCKS > 254
#error "SLOW_CLOCKS out of range"
#endif

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

static volatile int8_t direction;       // 1 = step formward, 0 = step backward
static volatile uint16_t steps;         // number of steps to make
static volatile uint8_t clocks;         // clocks per step, the update frequency
static volatile uint8_t phase=0;        // current motor phase

ISR(TIMER0_COMPA_vect)
{
    if (!steps)
    {
        TIMSK0 = 0;                     // disable interrupt
        TCCR0B = 0;                     // disable timer
        // coils off
        PORT(STEPPER_N) &= ~BIT(STEPPER_N);
        PORT(STEPPER_E) &= ~BIT(STEPPER_E);
        PORT(STEPPER_S) &= ~BIT(STEPPER_S);
        PORT(STEPPER_W) &= ~BIT(STEPPER_W);
        return;
    }

    // advance to next
    if (direction) phase++; else phase--;

    // energize coils of interest
    uint8_t p = phases[phase % STEPPER_PHASES];
    if (p & NORTH) PORT(STEPPER_N) |= BIT(STEPPER_N); else PORT(STEPPER_N) &= ~BIT(STEPPER_N);
    if (p & EAST) PORT(STEPPER_E) |= BIT(STEPPER_E); else PORT(STEPPER_E) &= ~BIT(STEPPER_E);
    if (p & SOUTH) PORT(STEPPER_S) |= BIT(STEPPER_S); else PORT(STEPPER_S) &= ~BIT(STEPPER_S);
    if (p & WEST) PORT(STEPPER_W) |= BIT(STEPPER_W); else  PORT(STEPPER_W) &= ~BIT(STEPPER_W);

    OCR0A = TCNT0+clocks-1;             // schedule next interrupt

    steps--;
    if (steps < STOP_STEPS)             // if near the end
    {
        uint8_t c = SLOW_CLOCKS - steps;
                                        // maybe decelerate
        if (clocks < c) clocks=c;
    }
    else                                // else maybe accelerate
        if (clocks > FAST_CLOCKS) clocks--;
}

// Disable stepper driver and release inputs.
void disable_stepper(void)
{
    TIMSK0 = 0;
    TCCR0B = 0;
    TIFR0 = 0xff;                       // clear pending interrupts
    PORT(STEPPER_N) &= ~BIT(STEPPER_N); // coils off
    PORT(STEPPER_E) &= ~BIT(STEPPER_E);
    PORT(STEPPER_S) &= ~BIT(STEPPER_S);
    PORT(STEPPER_W) &= ~BIT(STEPPER_W);
    DDR(STEPPER_N) &= ~BIT(STEPPER_N);  // set as inputs
    DDR(STEPPER_E) &= ~BIT(STEPPER_E);
    DDR(STEPPER_S) &= ~BIT(STEPPER_S);
    DDR(STEPPER_W) &= ~BIT(STEPPER_W);
}

// Enable stepper driver
void enable_stepper(void)
{
    disable_stepper();
    DDR(STEPPER_N) |= BIT(STEPPER_N);   // set as outputs
    DDR(STEPPER_E) |= BIT(STEPPER_E);
    DDR(STEPPER_S) |= BIT(STEPPER_S);
    DDR(STEPPER_W) |= BIT(STEPPER_W);
}

// Given direction 1=forward or 0=backward, and number of steps, (re)start the
// stepper, or stop it if steps == 0. Returns ASAP, the stepper runs in the
// background. Maximum steps is 65535, you're expected to keep calling this
// function if you don't want the stepper to stop.
// This function WILL turn interrupts on!!
void run_stepper(uint8_t d, uint16_t s)
{
    cli();
    if (TIMSK0)
    {
        if (!s || d==direction)
        {
            // already going in the right direction
            if (s > STOP_STEPS) steps = s;
            else if (steps > STOP_STEPS) steps = STOP_STEPS;
            // else it's already almost done
            sei();
            return;
        }
        // else we have to go the other way, stop first
        if (steps > STOP_STEPS) steps=STOP_STEPS;
        sei();                          // here we have to turn interrupts on
        while (TIMSK0);                 // wait for it...
    } else
    {
        sei();
        if (!s) return;
    }
    direction = d;                      // Set direction
    steps = s;                          // Set steps
    clocks = SLOW_CLOCKS;               // Start slow
    TCCR0B = 0;                         // Stop timer
    TCNT0 = 0;                          // Count from now
    OCR0A = 1;                          // Interrupt immediately
    TIFR0 = 0xff;                       // But not yet
    TCCR0A = 0;                         // Normal mode
    TCCR0B = 5;                         // Run timer at F_CPU/1024
    TIMSK0 = 2;                         // Enable OCIE0A interrupt
}

// Returns true if stepper is currently running, 0 if stopped.
int8_t check_stepper(void)
{
    return TIMSK0?1:0;
}
