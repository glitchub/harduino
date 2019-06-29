#define LED GPIO13

// Harduino rng demo
int main(void)
{
    OUT_GPIO(LED);
    SET_GPIO(LED);
    // init drivers and threads
    sei();
    init_ticks();           // this should be first
    init_serial();
    init_rng();
    sleep_ticks(1200);      // let the RNG initialize
    printf("start\n");
    while(1)
    {
        uint16_t r = get_rng();
        printf( "%04X ", r);
        for (uint16_t x=0x8000; x; x>>=1) printf( "%c", (r&x)?'1':'0');
        printf("\n");
        //printf( "%04X\n", r);
    }
}
