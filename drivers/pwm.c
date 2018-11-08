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
    if (percent<0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        DDR(PWM0) &= NOBIT(PWM0);                   // input
        TCCR0A &= 0x3f;                             // disable COM0A
    } 
    else if (percent == 0)
    {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        DDR(PWM0) |= BIT(PWM0);                     // output
        TCCR0A &= 0x3f;                             // disable COM0A
    } else if (percent >= 100)
    {
        PORT(PWM0) |= BIT(PWM0);                    // high
        DDR(PWM0) |= BIT(PWM0);                     // output
        TCCR0A &= 0x3f;                             // disable COM0A
    } else {
        PORT(PWM0) &= NOBIT(PWM0);                  // low
        DDR(PWM0) |= BIT(PWM0);                     // output
        TCCR0A |= 0x80;                             // set COM0A non-inverting mode
        OCR0A=(uint8_t)((256*percent)/100)-1;       // set pulse percent
    }   
    if (!(TCCR0A & 0xf0))                           // if pwms not used
    {
        TCCR0B = 0;                                 // turn off the clock              
    } else 
    {
        TCCR0A |= 3;                                // otherwise fast pwm
        TCCR0B = 1;                                 // at fastest clock
    }    
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
    } 
    else if (percent == 0)
    {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        DDR(PWM1) |= BIT(PWM1);                     // output
        TCCR0A &= 0xcf;                             // disable COM0B
    } else if (percent >= 100)
    {
        PORT(PWM1) |= BIT(PWM1);                  // high
        DDR(PWM1) |= BIT(PWM1);                     // output
        TCCR0A &= 0xcf;                             // disable COM0B
    } else {
        PORT(PWM1) &= NOBIT(PWM1);                  // low
        DDR(PWM1) |= BIT(PWM1);                     // output
        TCCR0A |= 0x20;                             // COM0B non-inverting mode
        OCR0B=(uint8_t)((256*percent)/100)-1;       // set pulse percent
    }    
    if (!(TCCR0A & 0xf0))                           // if pwms not used
    {
        TCCR0B = 0;                                 // turn off the clock              
    } else 
    {
        TCCR0A |= 3;                                // otherwise use fast pwm
        TCCR0B = 1;                                 // at fastest speed
    }    
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
        TCCR1A &= 0x3f;                             // disable COM1A
    } 
    else if (percent == 0)
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        DDR(PWM2) |= BIT(PWM2);                     // output
        TCCR1A &= 0x3f;                             // disable COM1A
    } else if (percent >= 100)
    {
        PORT(PWM2) |= BIT(PWM2);                    // high
        DDR(PWM2) |= BIT(PWM2);                     // output
        TCCR1A &= 0x3f;                             // disable COM1A
    } else 
    {
        PORT(PWM2) &= NOBIT(PWM2);                  // low
        DDR(PWM2) |= BIT(PWM2);                     // output
        TCCR1A |= 0x80;                             // COM1A non-inverting mode
        OCR1A=(uint8_t)((256*percent)/100)-1;       // set pulse percent
    }
    if (!(TCCR1A & 0xf0))                           // if pwms not used
    {
        TCCR1B = 0;                                 // turn off the clock              
    } else 
    {
        TCCR1A |= 1;                                // otherwise use 8-bit fast pwm
        TCCR1B = 9;                                 // at fastest clock
    }    
}
    
// Configure PWM3 output from 0 to100 percent, or disable completely if
// percent < 0.
void set_pwm3(char percent)
{
    if (percent < 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        DDR(PWM3) &= NOBIT(PWM3);                   // input
        TCCR1A &= 0xcf;                             // disable COM1B
    } 
    else if (percent == 0)
    {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        DDR(PWM3) |= BIT(PWM3);                     // output
        TCCR1A &= 0xcf;                             // disable COM1b
    } else if (percent >= 100)
    {
        PORT(PWM3) |= BIT(PWM3);                    // high
        DDR(PWM3) |= BIT(PWM3);                     // output
        TCCR1A &= 0xcf;                             // disable COM1B
    } else {
        PORT(PWM3) &= NOBIT(PWM3);                  // low
        DDR(PWM3) |= BIT(PWM3);                     // output
        TCCR1A |= 0x20;                             // set COM1A non-inverting mode
        OCR1B=(uint8_t)((256*percent)/100)-1;       // set pulse percent
    }
    if (!(TCCR1A & 0xf0))                           // if pwms not used
    {
        TCCR1B = 0;                                 // turn off the clock              
    } else 
    {
        TCCR1A |= 1;                                // otherwise use 8-bit fast pwm
        TCCR1B = 9;                                 // at fastest clock
    }    
}
#endif
