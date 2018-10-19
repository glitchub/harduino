// Stepper motor demo

#define LED GPIO13                              // on-board LED

static int8_t pressed(void)
{
    static int8_t was=0;
    int8_t hi=PIN(GPIOA5) & BIT(GPIOA5);
    if (hi != was)
    {
        was=hi;
        waitmS(20);
    }
    return !hi;
}

FILE *serial;

int main(void)
{
    enable_ticks(0);
    serial = enable_serial(115200UL);           // serial I/O
    enable_stepper();
    sei();

    DDR(LED) |= BIT(LED);                       // Make the LED an output

    PORT(GPIOA5) |= BIT(GPIOA5);                // pull-up on GPIOA5
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
            if (a <= 14) run_stepper(0, 2<<(14-a) );
            else if (a >= 17) run_stepper(1, 2<<(a-17) );
            else run_stepper(0, 0);
        }
        while (pressed());                      // wait for button release
        PORT(LED) &= ~BIT(LED);                 // toggle the LED
        while (!pressed())                      // while button not pressed
        {
            run_stepper(0,65535);
            t=get_ticks()+5000;
            while (!expired(t)) if (pressed()) break;
            run_stepper(1,65535);
            t=get_ticks()+5000;
            while (!expired(t)) if (pressed()) break;
        }
        run_stepper(0, 0);
    }
}
