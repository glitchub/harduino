// Unipolar stepper motor driver, using TIMER0.

#define FMAX ((MHZ*1000000)/1024)   // max stepper frequency is CPU clock / 1024
#define FMIN (FMAX/256)             // min stepper frequency is MAX/256

#ifndef STEPPER_FAST_HZ
#define STEPPER_FAST_HZ 1500
#endif
#if (STEPPER_FAST_HZ > FMAX)
#error "STEPPER_FAST_HZ out of range"
#endif

#ifndef STEPPER_SLOW_HZ
#define STEPPER_SLOW_HZ 500
#endif
#if (STEPPER_SLOW_HZ < FMIN) || (STEPPER_SLOW_HZ > STEPPER_FAST_HZ)
#error "STEPPER_SLOW_HZ out of range"
#endif

// Timer ticks corresponding to minimum and maximum stepper frequency (round up, to lower frequency)
#define FAST_CLOCKS ((FMAX+STEPPER_FAST_HZ-1)/STEPPER_FAST_HZ)
#define SLOW_CLOCKS ((FMAX+STEPPER_SLOW_HZ-1)/STEPPER_SLOW_HZ)

// Steps required to decelerate the stepper from fast to slow
#define STOP_STEPS (SLOW_CLOCKS-FAST_CLOCKS)

// Coil GPIOs
#if !defined(STEPPER_N) || !defined(STEPPER_E) || !defined(STEPPER_S) || !defined(STEPPER_W)
#error "Must define pins STEPPER_N, STEPPER_E, STEPPER_S, and STEPPER_W"
#endif

#define NORTH 1
#define EAST 2
#define SOUTH 4
#define WEST 8

#ifdef STEPPER_FULL    // full step
static uint8_t phases[4] = { NORTH|EAST, SOUTH|EAST, SOUTH|WEST, NORTH|WEST };
#else                  // half step
static uint8_t phases[8] = { NORTH, NORTH|EAST, EAST, SOUTH|EAST, SOUTH, SOUTH|WEST, WEST, NORTH|WEST };
#endif

static volatile bool forward;    // 1 = step forward, 0 = step backward
static volatile uint16_t steps;  // number of steps to make
static volatile uint8_t clocks;  // clocks per step, the update frequency
static volatile uint8_t phase=0; // current motor phase

ISR(TIMER0_COMPA_vect)
{
    if (!steps--)
    {
        TIMSK0 = 0;         // disable interrupt
        TCCR0B = 0;         // disable timer
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
    uint8_t p = phases[phase & (sizeof phases - 1)];
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
    OUT_GPIO(STEPPER_N);    // set as outputs
    OUT_GPIO(STEPPER_E);
    OUT_GPIO(STEPPER_S);
    OUT_GPIO(STEPPER_W);
}

// Start the stepper forward or backward
void start_stepper(int16_t s)
{
    bool f=1;               // forward if positive
    if (s < 0) f=0, s=-s;   // backward if negative
    cli();
    if (TIMSK0)
    {
        // motor is running
        if (!s || f==forward)
        {
            // already going in the right direction
            if (s > STOP_STEPS) steps = s;
            else if (steps > STOP_STEPS) steps = STOP_STEPS;
            sei();
            return;
        }
        // oops we have to go the other way
        if (steps > STOP_STEPS) steps=STOP_STEPS;
        sei();
        // spin here until motor stops...
#ifndef THREADED
        while (TIMSK0);
#else
        while (TIMSK0) yield();
#endif
    } else
    {
        sei();
        if (!s) return;
    }
    // motor is stopped
    forward = f;            // Set direction
    steps = s;              // Set steps
    clocks = SLOW_CLOCKS;   // Start at slowest speed
    TCCR0B = 0;             // Stop timer
    TCNT0 = 0;              // Count from now
    OCR0A = 1;              // Interrupt soon
    TIFR0 = 0xff;           // But not yet
    TCCR0A = 0;             // Normal mode
    TCCR0B = 5;             // Run timer at clock/1024
    TIMSK0 = 2;             // Enable OCIE0A interrupt
}

// start the stepper and wait for it to stop
void run_stepper(int16_t s)
{
    start_stepper(s);
#ifdef THREADED
    while (running_stepper()) yield();
#else
    while (running_stepper());
#endif
}

