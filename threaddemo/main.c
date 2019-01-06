// Harduino thread demo
#define LED GPIO13                              // on-board LED

extern uint8_t ticker_stack[];                  // from ticks.c

// poll dht11 every 2 seconds and update global variables
static uint8_t degrees, humidity;
static uint8_t dht11_stack[80];
static void __attribute__((noreturn)) dht11(void)
{
    init_dht11();                               // initialize
    sleep_ticks(1000);                          // allow one second to power on
    while(1)
    {
        get_dht11(&degrees, &humidity);         // update vars
        sleep_ticks(2000);                      // every two seconds
    }
}

static semaphore pwm_mutex=available(1);        // mutex, initially available
static uint8_t pwmleds_stack[80];
static void __attribute__((noreturn)) pwmleds(void)
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
static uint8_t console_stack[128];
static void __attribute__((noreturn)) console(void)
{
    FILE *serial = init_serial(115200UL);
    while(1)
    {
        static char cl[32] = "";                    // command buffer, make it static!
        uint8_t cln=0;                              // number of chars
        fputs("threaddemo> ", serial);
        while(1)
        {
            char c=fgetc(serial);                   // wait for a char
            switch(c)
            {
                case '\r':
                    while (cln && cl[cln-1]==' ') cln--;
                    cl[cln]=0;                      // zero terminate
                    fputs("\r\n",serial);
                    goto parse;                     // go parse it

                case '\b':
                    if (cln)                        // if we have some chars
                    {
                        cln--;                      // backspace
                        fputs("\b \b", serial);
                    }
                    break;

                case ' ':
                    if (!cln) break;                // no leading spaces
                    // fall thru

                case '!' ... '~':                   // printable
                    if (cln < sizeof(cl)-2)         // if it will fit
                    {
                        cl[cln++]=c;                // append it
                        fputc(c, serial);
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
            fprintf(serial, "Unused stack bytes:\r\n"
                            "  ticker : %d\r\n"
                            "  dht11  : %d\r\n"
                            "  console: %d\r\n"
                            "  pwmleds: %d\r\n",
                            stackspace(ticker_stack),
                            stackspace(dht11_stack),
                            stackspace(console_stack),
                            stackspace(pwmleds_stack));
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
                    fprintf(serial, "pwmleds thread is currently %s\r\n", hasmutex?"stopped":"running");
                    fprintf(serial, "TCCR0A=%02X TCCR0B=%02X OCR0A=%02X OCR0B=%02X\r\n", TCCR0A, TCCR0B, OCR0A, OCR0B);
                    fprintf(serial, "TCCR1A=%02X TCCR1B=%02X OCR1A=%04X OCR1B=%04X ICR1=%04X\r\n", TCCR1A, TCCR1B, OCR1A, OCR1B, ICR1);
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
                    fprintf(serial, "setting pwm freq = %u, actual = %u\r\n", freq, set_timer1_freq(freq));
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
                    fprintf(serial, "Setting pwm %d = %d\r\n", pwm, width);
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
            ng: fprintf(serial,"Invalid pwm command\r\n");
        }
        else if (!strcmp(tok,"fuses"))
        {
            cli();
            uint8_t lo = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
            uint8_t hi = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
            uint8_t ex = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
            uint8_t lock = boot_lock_fuse_bits_get(GET_LOCK_BITS);
            sei();
            fprintf(serial, "Fuses:\r\n"
                            "  Low     : %02X\r\n"
                            "  High    : %02X\r\n"
                            "  Extended: %02X\r\n"
                            "  Lock    : %02X\r\n",
                            lo, hi, ex, lock);
        }
        else if (!strcmp(tok, "temp"))
        {
            fprintf(serial, "Temperature: %dC (%dF)\r\n"
                            "Humidity   : %d%%\r\n",
                            degrees, ((degrees*9)/5)+32, humidity);
        }
        else if (!strcmp(tok, "uptime"))
        {
            uint32_t t=get_ticks();
            fprintf(serial, "Uptime: %ld.%03d seconds\r\n", t/1000, (int)(t%1000));
        }
        else
        {
            fprintf(serial, "Unknown command: '%s'\r\n"
                            "Try:\r\n"
                            "  temp                 -- show temperature and humidty\r\n"
                            "  pwm <0-3> <0-100>    -- set pwm output width 0-255\r\n"
                            "  pwm freq <1-16250>   -- set pwm2/3 frequency\r\n"
                            "  pwm run|stop|step    -- control pwmleds thread\r\n"
                            "  pwm status           -- show current pwm status\r\n"
                            "  pwm sync             -- sync pwm outputs\r\n"
                            "  fuses                -- show fuse bits\r\n"
                            "  uptime               -- show uptime in seconds\r\n"
                            "  stacks               -- show unused stack for known threads\r\n",
                            tok);
        }
    }
}

int main(void)
{
    // init drivers and threads
    sei();
    init_ticks(); // this should be first
    init_thread(console, console_stack, sizeof console_stack);
    init_thread(dht11, dht11_stack, sizeof dht11_stack);
    init_thread(pwmleds, pwmleds_stack, sizeof pwmleds_stack);

    // Enable sleep when all threads are suspended
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    // Just indicate we're alive
    DDR(LED) |= BIT(LED);           // Make the LED an output
    uint32_t next=get_ticks();
    while (1)
    {
        sleep_until(next+=200);     // every 200 mS
        PIN(LED) = BIT(LED);
    }
}
