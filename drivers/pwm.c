// PWM generation, via TIMER0 and TIMER1. The work is done by the hardware, all
// we need to do is configure it.
//
// The PWMx definitions are in the board-specific header file (e.g.  uno_r3.h)
// and map the PWM outputs to specific GPIO pins.

#if !defined(USE_PWM_TIMER0) && !defined(USE_PWM_TIMER1)
#error "Must define USE_PWM_TIMER0 or USE_PWM_TIMER1 (or both)"
#endif

#ifdef USE_PWM_TIMER0
// Configure PWM0 width 0 to 255, or disable output completely < 0.
void set_pwm0(int16_t width)
{
    if (width < 0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        DDR(PWM0) &= NOBIT(PWM0);                   // input
        TCCR0A &= 0x3F;                             // disable COM0A
        if (!(DDR(PWM1) & BIT(PWM1))) TCCR0B=0;     // and clock if pwm0 is also disabled
        return;
    }

    if (width == 0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        TCCR0A &= 0x3F;                             // disable COM0A
    } else if (width >= 255)
    {
        PORT(PWM0) |= BIT(PWM0);                    // high
        TCCR0A &= 0x3F;                             // disable COM0A
    } else
    {
        OCR0A = width;                              // set pulse width
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        TCCR0A |= 0x83;                             // COM0A non-inverting mode, fast pwm
        TCCR0B = 2;                                 // at /8 clock
    }
    DDR(PWM0) |= BIT(PWM0);                         // output
}

// Configure PWM1 width 0 to 255, or disable output completely < 0.
void set_pwm1(int16_t width)
{
    if (width < 0)
    {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        DDR(PWM1) &= NOBIT(PWM1);                   // input
        TCCR0A &= 0xCF;                             // disable COM0B
        if (!(DDR(PWM0) & BIT(PWM0))) TCCR0B=0;     // turn off clock if pwm0 is also disabled
        return;
    }

    if (width == 0)
    {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        TCCR0A &= 0xCF;                             // disable COM0B
    } else if (width >= 255)
    {
        PORT(PWM1) |= BIT(PWM1);                    // high
        TCCR0A &= 0xCF;                             // disable COM0B
    } else
    {
        OCR0B = width;                              // set width
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        TCCR0A |= 0x23;                             // COM0B non-inverting mode, fast pwm
        TCCR0B = 2;                                 // at /8 clock
    }
    DDR(PWM1) |= BIT(PWM1);                         // output
}
#endif

#ifdef USE_PWM_TIMER1
// remember current frequency
static uint16_t top=255;    // initially the same as timer 0
static uint8_t prescale=2;
static uint8_t width2;      // current pwm2 pulse width 0-255
static uint8_t width3;      // current pwm3 pulse width 0-255

// Set timer1 frequency, 1 to 62500Hz. Returns the actual configured frequency
// which due to rounding errors will usually be somewhat higher than the
// requested frequency. If frequency==0 then the clock is set to match timer0.
// Note Hz=F_CPU/(div*(ICR1+1)) and ICR1=(F_CPU/(Hz*div))-1. Existing PWM
// ratios are maintained.
uint16_t set_timer1_freq(uint16_t hz)
{
    uint16_t div[] = { 0, 1, 8, 64, 256, 1024 }; // map prescaler index to divider value

    if (!hz) top = 255, prescale = 2; // restore default
    else
    {
        // find the highest prescaler which can provide requested frequency
        int32_t t;
        for (prescale = 5; prescale; prescale--)
        {
            t=(F_CPU/((uint32_t)hz*div[prescale]))-1;
            if (t >= 255) break;
        }
        if (!prescale) t=255, prescale=1; // too high
        top=t;
    }

    // update pwms if currently running
    if (TCCR1B)
    {
        TCCR1B=0;
        TCNT1=0;
        ICR1 = top;
        if (width2) OCR1A = ((uint32_t)top*width2)/255;
        if (width3) OCR1B = ((uint32_t)top*width3)/255;
        TCCR1B = 0x18|prescale; // CTC IRC1 with prescale
    }

    // return the actual frequency
    return F_CPU/((uint32_t)div[prescale]*(top+1));
}

// Configure PWM2 width 0 to 255, or disable output completely < 0.
// Set PWM2 pulse width as (255/width)*100% 
void set_pwm2(int16_t width)
{
    width2 = 0;
    if (width < 0)
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        DDR(PWM2) &= NOBIT(PWM2);                   // input
        TCCR1A &= 0x3F;                             // disable COM1A
        if (!(DDR(PWM3) & BIT(PWM3))) TCCR1B=0;     // turn off clock if pwm3 is also disabled
        return;
    }

    if (width == 0)
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        TCCR1A &= 0x3F;                             // disable COM1A
    } else if (width >= 255)
    {
        PORT(PWM2) |= BIT(PWM2);                    // high
        TCCR1A &= 0x3F;                             // disable COM1A
    } else
    {
        width2 = width;                             // remember pwm2 width
        ICR1 = top;
        OCR1A = ((uint32_t)top*width2)/255;         // set pulse width as a fraction of top
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        TCCR1A |= 0x82;                             // COM1A non-inverting
        TCCR1B = 0x18|prescale;                     // CTC ICR1 with prescale
    }
    DDR(PWM2) |= BIT(PWM2);                         // output
}

// Configure PWM3 width 0 to 255, or disable output completely < 0.
void set_pwm3(int16_t width)
{
    width3 = 0;
    if (width < 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        DDR(PWM3) &= NOBIT(PWM3);                   // input
        TCCR1A &= 0xCF;                             // disable COM1B
        if (!(DDR(PWM2) & BIT(PWM2))) TCCR1B=0;     // turn off clock if pwm2 is also disabled
        return;
    }

    if (width == 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        TCCR1A &= 0xCF;                             // disable COM1B
    } else if (width >= 255)
    {
        PORT(PWM3) |= BIT(PWM3);                    // high
        TCCR1A &= 0xCF;                             // disable COM1B
    } else
    {
        width3 = width;                             // remember pwm3 width
        ICR1 = top;
        OCR1B = ((uint32_t)top*width3)/255;         // set pulse width
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        TCCR1A |= 0x22;                             // COM1B non-inverting mode
        TCCR1B = 0x18|prescale;                     // CTC ICR1 with prescale
    }
    DDR(PWM3) |= BIT(PWM3);                         // output
}
#endif

#if defined(USE_PWM_TIMER0) && defined(USE_PWM_TIMER1)
// Sync timer0 and timer1 so PWM outputs go high at the same time.
void sync_pwm(void)
{
    GTCCR=0x81;                                     // Set TSM and PSRSYNC to halt timer 0 and 1
    TCNT0=0;
    TCNT1=0;
    GTCCR=0;                                        // let timers run
}
#endif
