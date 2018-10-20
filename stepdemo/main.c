// Stepper motor demo

#define LED GPIO13                              // on-board LED
#define BUTTON GPIOA5                           // push button

// Return true if button is pressed
static int8_t pressed(void)
{
    static int8_t was=0;
    int8_t hi=PIN(BUTTON) & BIT(BUTTON);
    if (hi != was)
    {
        was=hi;
        sleep_ticks(20);                        // sleep through button bounce
    }
    return !hi;
}

int main(void)
{
    enable_ticks(0);
    enable_stepper();
    sei();

    DDR(LED) |= BIT(LED);                       // Make the LED an output
    PORT(BUTTON) |= BIT(BUTTON);                // set pull-up on BUTTON input
    ADMUX = 0x24;                               // prepare for 8-bit A/D conversion on GPIOA4

    while(1)
    {
        PORT(LED) |= BIT(LED);
        while (pressed());                      // wait for button release
        uint32_t t = get_ticks()+100;
        while(!pressed())                       // while button not pressed
        {
            if (expired(t))
            {
                PIN(LED) |= BIT(LED);
                t+=100;
            }
            ADCSRA = 0xc4;                      // start conversion
            while (ADCSRA & 0x40);              // wait for complete
            uint8_t a = ADCH/8;                 // 0-31
            // 0-14 run forward, 7-31 run backward, 1 to 16384
            if (a <= 14) run_stepper(1<<(14-a));
            else if (a >= 17) run_stepper(-(1<<(a-17)));
            else run_stepper(0);                // or stop
        }
        while (pressed());                      // wait for button release
        PORT(LED) &= ~BIT(LED);                 // toggle the LED
        while (1)
        {
            run_stepper(4096);
            while (check_stepper())  if (pressed()) goto out;
            t=get_ticks()+250;
            while (!expired(t)) if (pressed()) goto out;
            run_stepper(-4096);
            while (check_stepper())  if (pressed()) goto out;
            t=get_ticks()+250;
            while (!expired(t)) if (pressed()) goto out;
        }
        out:
        run_stepper(0);
    }
}
