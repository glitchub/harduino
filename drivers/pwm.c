// PWM generation, via TIMER0 and TIMER1. The work is done by the hardware, all
// we need to do is configure it.
//
// Note the PWMx definitions are in the board-specific header file (e.g.
// uno_r3.h) and map the PWM output to a specific GPIO pin.

#if !defined(USE_PWM_TIMER0) && !defined(USE_PWM_TIMER1)
#error "Must define USE_PWM_TIMER0 or USE_PWM_TIMER1 (or both)"
#endif

#ifdef USE_PWM_TIMER0
// Configure PWM0 output from 0 to 100 percent, or disable output completely
// if percent < 0.
void set_pwm0(char percent)
{
    if (percent < 0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        DDR(PWM0) &= NOBIT(PWM0);                   // input
        TCCR0A &= 0x3f;                             // disable COM0A
        if (!(DDR(PWM1) & BIT(PWM1))) TCCR0B=0;     // and clock if pwm0 is also disabled
        return;
    }

    if (percent == 0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        TCCR0A &= 0x3f;                             // disable COM0A
    } else if (percent >= 100)
    {
        PORT(PWM0) |= BIT(PWM0);                    // high
        TCCR0A &= 0x3f;                             // disable COM0A
    } else
    {
        OCR0A = (255*percent)/100;                  // set pulse percent
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        TCCR0A |= 0x80;                             // COM0A non-inverting mode
        TCCR0A |= 3;                                // fast pwm
        TCCR0B = 1;                                 // at fastest clock
    }
    DDR(PWM0) |= BIT(PWM0);                         // output
}

// Configure PWM1 output from 0 to100 percent, or disable completely if
// percent < 0.
void set_pwm1(char percent)
{
    if (percent < 0)
    {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        DDR(PWM1) &= NOBIT(PWM1);                   // input
        TCCR0A &= 0xcf;                             // disable COM0B
        if (!(DDR(PWM0) & BIT(PWM0))) TCCR0B=0;     // turn off clock if pwm0 is also disabled
        return;
    }

    if (percent == 0)
    {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        TCCR0A &= 0xcf;                             // disable COM0B
    } else if (percent >= 100)
    {
        PORT(PWM1) |= BIT(PWM1);                    // high
        TCCR0A &= 0xcf;                             // disable COM0B
    } else
    {
        OCR0B = (255*percent)/100;                  // set pulse percent
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        TCCR0A |= 0x20;                             // COM0B non-inverting mode
        TCCR0A |= 3;                                // fast pwm
        TCCR0B = 1;                                 // at fastest speed
    }
    DDR(PWM1) |= BIT(PWM1);                         // output
}
#endif

#ifdef USE_PWM_TIMER1
// Configure PWM2 output from 0 to100 percent, or disable completely if
// percent < 0.
void set_pwm2(char percent)
{
    if (percent < 0)
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        DDR(PWM2) &= NOBIT(PWM2);                   // input
        TCCR1A &= 0x3f ;                            // disable COM1A
        if (!(DDR(PWM3) & BIT(PWM3))) TCCR1B=0;     // turn off clock if pwm3 is also disabled
        return;
    }

    if (percent == 0)
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        TCCR1A &= 0x3f;                             // disable COM1A
    } else if (percent >= 100)
    {
        PORT(PWM2) |= BIT(PWM2);                    // high
        TCCR1A &= 0x3f;                             // disable COM1A
    } else
    {
        OCR1A = (255*percent)/100;                  // set pulse percent
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        TCCR1A |= 0x80;                             // COM1A non-inverting mode
        TCCR1A |= 1;                                // 8-bit fast pwm
        TCCR1B = 9;                                 // at fastest clock
    }
    DDR(PWM2) |= BIT(PWM2);                         // output
}

// Configure PWM3 output from 0 to 100 percent, or disable completely if
// percent < 0.
void set_pwm3(char percent)
{
    if (percent < 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        DDR(PWM3) &= NOBIT(PWM3);                   // input
        TCCR1A &= 0xcf;                             // disable COM1B
        if (!(DDR(PWM2) & BIT(PWM2))) TCCR1B=0;     // turn off clock if pwm2 is also disabled
        return;
    }

    if (percent == 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        TCCR1A &= 0xcf;                             // disable COM1B
    } else if (percent >= 100)
    {
        PORT(PWM3) |= BIT(PWM3);                    // high
        TCCR1A &= 0xcf;                             // disable COM1B
    } else
    {
        OCR1B = (255*percent)/100;                  // set pulse percent
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        TCCR1A |= 0x20;                             // COM1B non-inverting mode
        TCCR1A |= 1;                                // 8-bit fast pwm
        TCCR1B = 9;                                 // at fastest clock
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
