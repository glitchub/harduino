// Harduino thread demo
#define LED GPIO13                              // on-board LED

extern uint8_t ticket_stack[];                  // from ticks.c

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
    static const uint8_t phases[] PROGMEM = {0,1,8,25,100,25,8,1};
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
        sleep_ticks(40);
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
                            "  ticket : %d\r\n"
                            "  dht11  : %d\r\n"
                            "  console: %d\r\n"
                            "  pwmleds: %d\r\n",
                            stackspace(ticket_stack),
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
                    release_mutex(&pwm_mutex);  // release mutex (if not already)
                    hasmutex=0;                 // remember we don't have it anymore
                    continue;
                }
                if (!strcmp(tok,"stop"))
                {
                    // maybe grab mutex and hold it until 'run'
                    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
                    continue;
                }
                if (!strcmp(tok, "status"))
                {
                    fprintf(serial, "pwmleds thread is currently %s\r\n", hasmutex?"stopped":"running");
                    fprintf(serial, "TCCR0A=%02X TCCR0B=%02X OCR0A=%02X OCR0B=%02X\r\n", TCCR0A, TCCR0B, OCR0A, OCR0B);
                    fprintf(serial, "TCCR1A=%02X TCCR1B=%02X OCR1A=%04X OCR1B=%04X\r\n", TCCR1A, TCCR1B, OCR1A, OCR1B);
                    continue;
                }
                uint8_t pwm=(uint8_t)strtoul(tok,NULL,0);
                if (pwm <= 3)
                {
                    tok=strtok(NULL," ");
                    if (tok)
                    {
                        // maybe grab mutex and hold it until 'run'
                        if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
                        char percent=(char)strtol(tok,NULL,0);
                        fprintf(serial, "Setting pwm %d = %d\r\n", pwm, percent);
                        switch (pwm)
                        {
                            case 0: set_pwm0(percent); break;
                            case 1: set_pwm1(percent); break;
                            case 2: set_pwm2(percent); break;
                            case 3: set_pwm3(percent); break;
                        }
                        continue;
                    }
                }
            }
            fprintf(serial,"Invalid pwm command\r\n");
        }
        else if (!strcmp(tok,"timers"))
        {
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
                            "  pwm <0-3> <0-100>    -- set pwm output percentage\r\n"
                            "  pwm run|stop         -- run or stop the pwmleds thread\r\n"
                            "  pwm status           -- show current pwm status\r\n"
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
    init_ticks(); // this should be first
    init_thread(console, console_stack, sizeof console_stack);
    init_thread(dht11, dht11_stack, sizeof dht11_stack);
    init_thread(pwmleds, pwmleds_stack, sizeof pwmleds_stack);

    // Enable sleep when all threads are suspended
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sei();

    // Just indicate we're alive
    DDR(LED) |= BIT(LED);         // Make the LED an output
    while (1)
    {
        sleep_ticks(200);
        PIN(LED) = BIT(LED);
    }
}
