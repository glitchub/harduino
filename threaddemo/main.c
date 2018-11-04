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
        parse:
        if (!cl[0]) continue;
        if (!strcmp(cl,"stacks"))
        {
            fprintf(serial, "Unused stack bytes:\r\n"
                            "  ticket : %d\r\n"
                            "  dht11  : %d\r\n"
                            "  console: %d\r\n",
                            stackspace(ticket_stack), stackspace(dht11_stack), stackspace(console_stack));
        }
        else if (!strcmp(cl,"fuses"))
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
        else if (!strcmp(cl, "temp"))
        {
            fprintf(serial, "Temperature: %dC (%dF)\r\n"
                            "Humidity   : %d%%\r\n",
                            degrees, ((degrees*9)/5)+32, humidity);
        }
        else if (!strcmp(cl, "uptime"))
        {
            uint32_t t=get_ticks();
            fprintf(serial, "Uptime: %ld.%03d seconds\r\n", t/1000, (int)(t%1000));
        }
        else
        {
            fprintf(serial, "Unknown command: '%s'\r\n"
                            "Try:\r\n"
                            "  temp   -- temperature and humidty\r\n"
                            "  fuses  -- fuse bits\r\n"
                            "  uptime -- uptime in seconds\r\n"
                            "  stacks -- stack space for known threads\r\n",
                            cl);
        }
    }
}

int main(void)
{
    // init drivers and threads
    init_ticks(); // this should be first
    init_thread(console, console_stack, sizeof console_stack);
    init_thread(dht11, dht11_stack, sizeof dht11_stack);

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
