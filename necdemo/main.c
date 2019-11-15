// NEC IR receive demo, note this is non-threaded

#define LED GPIO13                              // on-board LED

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_serial();
    init_nec();

    pprintf("Waiting for IR...\n");
    while(true)
    {
        int8_t event;
        uint32_t key;

        sleep_ticks(100);
        TOG_GPIO(LED);
        if ((event=get_nec(&key))!=0)
            pprintf("%7s: vendor=%04X key=%02X check=%02X\n", (event>0)?"Press":"Release",NEC_VENDOR(key), NEC_KEY(key), NEC_CHECK(key));
    }
}
