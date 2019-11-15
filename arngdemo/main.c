#define LED GPIO13

THREAD(blink,80)
{
    OUT_GPIO(LED);
    while (true)
    {
        TOG_GPIO(LED);
        sleep_ticks(100);
    }
}

uint32_t get_entropy(void);
// Harduino rng demo
int main(void)
{
    init_arng();
    init_serial();
    start_threads();
    sleep_ticks(1000);      // let RNG gather entropy
    while(1)
    {
        pprintf("%08lX\n", get_arng());
        yield();
    }
}
