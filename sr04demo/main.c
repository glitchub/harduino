// Harduino pwm demo
#define LED GPIO13                              // on-board LED

// Use gpio04 for module trigger, and gpio05 form echo
gpio trig = {GPIO04},
     echo = {GPIO05};

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_serial();
    while(true)
    {
        sleep_ticks(250);
        TOG_GPIO(LED);
        printf("%d\n", get_sr04(&trig, &echo));
    }
}
