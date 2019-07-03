// LCD module demo

#define LED GPIO13                              // on-board LED

// The LCD controller is operated in write-only 4-bit mode and requires only 6
// signals from the CPU: D4, D5, D6, D7, E and RS. R/W must be grounded. 

static lcd glass = 
{
    .lines=2,
    .columns=16,
    .RS=(gpio[]){{GPIO02}},
    .E= (gpio[]){{GPIO03}},
    .D4=(gpio[]){{GPIO04}},
    .D5=(gpio[]){{GPIO05}},
    .D6=(gpio[]){{GPIO06}},
    .D7=(gpio[]){{GPIO07}},
};

static int8_t wlcd(int8_t argc, char *argv[])
{
    if (argc != 2) die("Usage: write 'string'\n");
    fprintf(&glass.handle, "\f%s\v", argv[1]);
    return 0;
}
COMMAND("lcd", NULL, "Write to LCD", wlcd);

THREAD(clock, 64)
{
    OUT_GPIO(LED);                              // Make the LED an output
    init_lcd(&glass);                           // 2x16 LCD module
    fprintf(&glass.handle,"\fHello\v");
    while(true)
    {
        TOG_GPIO(LED);                          // flash it
        uint32_t now = get_ticks()/1000;        // get current seconds    
        uint8_t d=now / 86400UL;
        uint8_t h=(now % 86400UL) / 3600;
        uint8_t m=(now % 3600) / 60;
        uint8_t s=now % 60;
        // print on second line
        fprintf(&glass.handle, "\f\n%02u:%02u:%02u:%02u\v", d, h, m, s);
        sleep_ticks(100);
    }
}    

int main(void)
{
    // init drivers
    sei();
    init_serial();                              // serial I/O
    start_threads();
    command(">");                               // go process commands 
}
