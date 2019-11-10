// Harduino pwm demo
#define LED GPIO13                              // on-board LED

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_serial();
    init_sr04();
    while(true)
    {
        sleep_ticks(250);
        TOG_GPIO(LED);
        printf("%d\n", get_sr04());
    }
}
