// Stepper motor demo

#define LED GPIO13      // on-board LED
#define BUTTON GPIO08   // button

int main(void)
{
    init_ticks();
    OUT_GPIO(LED);
    init_stepper();
    while(true)
    {
        TOG_GPIO(LED);
        run_stepper(4096);
        run_stepper(-4096);
    }
}
