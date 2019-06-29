// Harduino thread demo
#define LED GPIO13                              // on-board LED


// poll dht11 every 2 seconds and update global variables
static uint8_t degrees, humidity;
THREAD(dht11,80)
{
    static gpio dht11={GPIO02};                 // dht11 attached to GPIO02
    init_dht11(&dht11);                         // initialize
    sleep_ticks(1000);                          // allow one second to power on
    while(1)
    {
        get_dht11(&degrees, &humidity, &dht11); // update vars
        sleep_ticks(2000);                      // every two seconds
    }
}

static semaphore pwm_mutex=available(1);        // mutex, initially available
THREAD(pwmled,80)
{
    static const uint8_t phases[] PROGMEM = {0, 7, 22, 68, 206, 68, 22, 7};

    uint8_t phase=0;
    while (1)
    {
        suspend(&pwm_mutex); // block here while console holds the mutex
        set_pwm0(pgm_read_byte(&phases[phase]));
        set_pwm1(pgm_read_byte(&phases[(phase+1) % sizeof phases]));
        set_pwm2(pgm_read_byte(&phases[(phase+2) % sizeof phases]));
        set_pwm3(pgm_read_byte(&phases[(phase+3) % sizeof phases]));
        phase=(phase+1) % sizeof(phases);
        release(&pwm_mutex);
        sleep_ticks(60);
    }
}

// simple console interface
THREAD(console,128)
{
    init_serial();
    while(1)
    {
        static char cl[32] = "";                    // command buffer, make it static!
        uint8_t cln=0;                              // number of chars
        puts("threaddemo> ");
        while(1)
        {
            char c=getchar();                       // wait for a char
            switch(c)
            {
                case '\r':
                    while (cln && cl[cln-1]==' ') cln--;
                    cl[cln]=0;                      // zero terminate
                    puts("\n");
                    goto parse;                     // go parse it

                case '\b':
                    if (cln)                        // if we have some chars
                    {
                        cln--;                      // backspace
                        puts("\b \b");
                    }
                    break;

                case ' ':
                    if (!cln) break;                // no leading spaces
                    // fall thru

                case '!' ... '~':                   // printable
                    if (cln < sizeof(cl)-2)         // if it will fit
                    {
                        cl[cln++]=c;                // append it
                        putchar(c);
                    }
                    break;
            }
        }
        char *tok;
        parse:
        tok=strtok(cl," ");
        if (!tok) continue;
        if (!strcmp(tok,"stacks"))
        {
            debug_stacks();
        }
        else if (!strcmp(tok,"pwm"))
        {
            static char hasmutex=0;
            tok=strtok(NULL," ");
            if (tok)
            {
                if (!strcmp(tok,"run"))
                {
                    if (hasmutex) release(&pwm_mutex), hasmutex=0;
                    continue;
                }
                if (!strcmp(tok,"stop"))
                {
                    // maybe grab mutex and hold it until 'run'
                    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
                    continue;
                }
                if (!strcmp(tok,"step"))
                {
                    // release mutex and immediately grab it again, i.e. pwmleds runs once
                    if (hasmutex) release(&pwm_mutex);
                    suspend(&pwm_mutex);
                    hasmutex=1;
                    continue;
                }
                if (!strcmp(tok, "status"))
                {
                    printf("pwmleds thread is currently %s\n", hasmutex?"stopped":"running");
                    printf("TCCR0A=%02X TCCR0B=%02X OCR0A=%02X OCR0B=%02X\n", TCCR0A, TCCR0B, OCR0A, OCR0B);
                    printf("TCCR1A=%02X TCCR1B=%02X OCR1A=%04X OCR1B=%04X ICR1=%04X\n", TCCR1A, TCCR1B, OCR1A, OCR1B, ICR1);
                    continue;
                }
                if (!strcmp(tok, "sync"))
                {
                    sync_pwm();
                    continue;
                }
                if (!strcmp(tok, "freq"))
                {
                    tok=strtok(NULL, " ");
                    if (!tok) goto ng;
                    uint16_t freq=(uint16_t)strtoul(tok,NULL,0);
                    printf("setting pwm freq = %u, actual = %u\n", freq, set_timer1_freq(freq));
                    continue;
                }
                uint8_t pwm=(uint8_t)strtoul(tok,NULL,0);
                if (pwm <= 3)
                {
                    tok=strtok(NULL," ");
                    if (!tok) goto ng;
                    // maybe grab mutex and hold it until 'run'
                    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
                    int16_t width=(int16_t)strtol(tok,NULL,0);
                    printf("Setting pwm %d = %d\n", pwm, width);
                    switch (pwm)
                    {
                        case 0: set_pwm0(width); break;
                        case 1: set_pwm1(width); break;
                        case 2: set_pwm2(width); break;
                        case 3: set_pwm3(width); break;
                    }
                    continue;
                }
            }
            ng: printf("Invalid pwm command\n");
        }
        else if (!strcmp(tok,"fuses"))
        {
            cli();
            uint8_t lo = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
            uint8_t hi = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
            uint8_t ex = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
            uint8_t lock = boot_lock_fuse_bits_get(GET_LOCK_BITS);
            sei();
            printf("Fuses:\n"
                            "  Low     : %02X\n"
                            "  High    : %02X\n"
                            "  Extended: %02X\n"
                            "  Lock    : %02X\n",
                            lo, hi, ex, lock);
        }
        else if (!strcmp(tok, "temp"))
        {
            printf("Temperature: %dC (%dF)\n"
                            "Humidity   : %d%%\n",
                            degrees, ((degrees*9)/5)+32, humidity);
        }
        else if (!strcmp(tok, "uptime"))
        {
            uint32_t t=get_ticks();
            printf("Uptime: %ld.%03d seconds\n", t/1000, (int)(t%1000));
        }
        else
        {
            printf("Unknown command: '%s'\n"
                            "Try:\n"
                            "  temp                 -- show temperature and humidty\n"
                            "  pwm <0-3> <0-100>    -- set pwm output width 0-255\n"
                            "  pwm freq <1-16250>   -- set pwm2/3 frequency\n"
                            "  pwm run|stop|step    -- control pwmleds thread\n"
                            "  pwm status           -- show current pwm status\n"
                            "  pwm sync             -- sync pwm outputs\n"
                            "  fuses                -- show fuse bits\n"
                            "  uptime               -- show uptime in seconds\n"
                            "  stacks               -- show unused stack for known threads\n",
                            tok);
        }
    }
}

int main(void)
{
    // start all threads
    start_threads();

    // this is a thread too, just blink
    OUT_GPIO(LED);                  // Make the LED an output
    uint32_t next=get_ticks();
    while (1)
    {
        sleep_until(next+=200);     // every 200 mS
        TOG_GPIO(LED);
    }
}
