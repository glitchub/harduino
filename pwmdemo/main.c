// Harduino pwm demo
#define LED GPIO13                              // on-board LED

static semaphore pwm_mutex=available(1);        // mutex, initially available
THREAD(pwmled,80)
{
    static const uint8_t phases[] = {0, 7, 22, 68, 206, 68, 22, 7};

    uint8_t phase=0;
    while (1)
    {
        suspend(&pwm_mutex); // block here while console holds the mutex
        set_pwm0(phases[phase]);
        set_pwm1(phases[(phase+1) % sizeof phases]);
        set_pwm2(phases[(phase+2) % sizeof phases]);
        set_pwm3(phases[(phase+3) % sizeof phases]);
        phase=(phase+1) % sizeof(phases);
        release(&pwm_mutex);
        sleep_ticks(60);
    }
}

static char hasmutex=0;
// release mutex if currently grabbede
static int8_t run(int8_t argc, char **argv)
{
    if (hasmutex) release(&pwm_mutex), hasmutex=0;
    return 0;
}
COMMAND("run", NULL, "start PWMs", run);

// grab mutex if not grabbed
static int8_t stop(int8_t argc, char **argv)
{
    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
    return 0;
}
COMMAND("stop", NULL, "stop PWMs", stop);

// release mutex and immediately grab it again, i.e. pwmleds runs once
static int8_t step(int8_t argc, char **argv)
{
    if (hasmutex) release(&pwm_mutex);
    suspend(&pwm_mutex);
    hasmutex=1;
    return 0;
}
COMMAND("step", NULL, "step PWMs", step);

static int8_t status(int8_t argc, char **argv)
{
    printf("pwmleds thread is currently %s\n", hasmutex?"stopped":"running");
    printf("TCCR0A=%02X TCCR0B=%02X OCR0A=%02X OCR0B=%02X\n", TCCR0A, TCCR0B, OCR0A, OCR0B);
    printf("TCCR1A=%02X TCCR1B=%02X OCR1A=%04X OCR1B=%04X ICR1=%04X\n", TCCR1A, TCCR1B, OCR1A, OCR1B, ICR1);
    return 0;
}
COMMAND("status", NULL, "show PWM status", status);

static int8_t sync(int8_t argc, char **argv)
{
    sync_pwm();
    return 0;
}
COMMAND("sync", NULL, "sync PWM states", sync);

static int8_t freq(int8_t argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: freq 1-16250\n");
        return 1;
    }
    uint16_t freq=(uint16_t)strtoul(argv[1],NULL,0);
    printf("setting pwm freq = %u, actual = %u\n", freq, set_timer1_freq(freq));
    return 0;
}
COMMAND("freq",  NULL, "set PWM 2/3 frequency", freq);

static int8_t width(int8_t argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: width pwm percent\n");
        return 1;
    }
    uint8_t pwm=(uint8_t)strtoul(argv[1],NULL,0);
    int16_t width=(int16_t)strtol(argv[2],NULL,0);
    printf("Setting pwm %d = %d\n", pwm, width);
    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
    switch (pwm)
    {
        case 0: set_pwm0(width); break;
        case 1: set_pwm1(width); break;
        case 2: set_pwm2(width); break;
        case 3: set_pwm3(width); break;
    }
    return 0;
}
COMMAND("width", NULL, "set a PWM pulse width percent", width);

THREAD(blink,80)
{
    OUT_GPIO(LED);                  // Make the LED an output
    while(true)
    {
        TOG_GPIO(LED);
        sleep_ticks(200);
    }
}

int main(void)
{
    init_serial();
    start_threads();
    command(">");
}
