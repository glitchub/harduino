// A2D demo, just show all 15 channels
#define LED GPIO13                              // on-board LED

int main(void)
{
    OUT_GPIO(LED);
    sei();
    init_ticks();
    init_serial();

    while(true)
    {
        TOG_GPIO(LED);
        for (int i=0; i<8; i++) printf("%0d=%03X ",i,get_a2d((struct a2d []){{i, false, false}}));
        printf("\n");
        sleep_ticks(250);
    }
}
