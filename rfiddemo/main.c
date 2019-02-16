// Harduino MFRC522 card reader demo
int main(void)
{
    sei();
    init_ticks(); // this should be first

    // Enable sleep when all threads are suspended
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    // init the console
    FILE *serial = init_serial(115200UL);

    if (!init_mfrc522())
    {
        fprintf(serial, "Failed to initialize card reader.\r\n");
        while(1) sleep_ticks(1000000000);
    }
    fprintf(serial, "Ready for card swipe!\r\n");
    while(1)
    {
        uint8_t uid[10];
        int8_t got=get_mfrc522(uid);
        switch(got)
        {
            // 0 means no card, that's normal so say nothing
            case 0:
                break;

            // 4, 7, or 10 is UID length in bytes, report it.
            case 4:
            case 7:
            case 10:
                fprintf(serial, "Got UID ");
                for (int i=0; i < got; i++) fprintf(serial, "%02X", uid[i]);
                fprintf(serial,"\r\n");
                break;

            // Error status is also normal and in real life would not report
            // it, but for this demo we'll be verbose.
            default:
                fprintf(serial, "Got error %d\r\n", got);
                break;
        }
    }
}
