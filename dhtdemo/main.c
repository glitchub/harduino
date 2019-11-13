// PWM demo
#define LED GPIO13                              // on-board LED

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_serial();
    while(true)
    {
        sleep_ticks(250);
        TOG_GPIO(LED);
        uint8_t dc, rh, err;
        err=get_dht11(&dc, &rh, (gpio []){{GPIO04}});
        if (err) pprintf("Error %d\n", err);
        else pprintf("%dC, %d%% humidity\n", dc, rh);
    }
}
