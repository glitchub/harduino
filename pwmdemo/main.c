// Harduino pwm demo
#define LED GPIO13                              // on-board LED

static semaphore pwm_mutex=available(1);        // mutex, initially available
THREAD(pwmled,65)
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
COMMAND(run, NULL, "start PWMs")
{
    if (hasmutex) release(&pwm_mutex), hasmutex=0;
}

// grab mutex if not grabbed
COMMAND(stop, NULL, "stop PWMs")
{
    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
}

// release mutex and immediately grab it again, i.e. pwmleds runs once
COMMAND(step, NULL, "step PWMs")
{
    if (hasmutex) release(&pwm_mutex);
    suspend(&pwm_mutex);
    hasmutex=1;
}

COMMAND(status, NULL, "show PWM status")
{
    pprintf("pwmleds thread is currently %s\n", hasmutex?"stopped":"running");
    pprintf("TCCR0A=%02X TCCR0B=%02X OCR0A=%02X OCR0B=%02X\n", TCCR0A, TCCR0B, OCR0A, OCR0B);
    pprintf("TCCR1A=%02X TCCR1B=%02X OCR1A=%04X OCR1B=%04X ICR1=%04X\n", TCCR1A, TCCR1B, OCR1A, OCR1B, ICR1);
}

COMMAND(sync, NULL, "sync PWM states")
{
    sync_pwm();
}

COMMAND(freq,  NULL, "set PWM 2/3 frequency")
{
    if (argc != 2) die("Usage: freq 1-16250\n");
    uint16_t freq=(uint16_t)strtoul(argv[1],NULL,0);
    pprintf("setting pwm freq = %u, actual = %u\n", freq, set_timer1_freq(freq));
}

COMMAND(width, NULL, "set a PWM pulse width percent")
{
    if (argc != 3) die("Usage: width pwm percent\n");
    uint8_t pwm=(uint8_t)strtoul(argv[1],NULL,0);
    int16_t width=(int16_t)strtol(argv[2],NULL,0);
    pprintf("Setting pwm %d = %d\n", pwm, width);
    if (!hasmutex) suspend(&pwm_mutex), hasmutex=1;
    switch (pwm)
    {
        case 0: set_pwm0(width); break;
        case 1: set_pwm1(width); break;
        case 2: set_pwm2(width); break;
        case 3: set_pwm3(width); break;
    }
}

THREAD(blink,65)
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
    pprintf("PWM demo\n");
    start_threads();
    command(">");
}
