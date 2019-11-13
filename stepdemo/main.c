// Stepper motor demo

#define LED GPIO13      // on-board LED

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_stepper();
    while(true)
    {
        TOG_GPIO(LED);
        run_stepper(4096);
        run_stepper(-4096);
    }
}
