// Harduino MFRC522 card reader demo

#define LED GPIO13

int main(void)
{
    OUT_GPIO(LED);
    init_ticks();
    init_serial();
    if (!init_mfrc522())
    {
        pprintf("Failed to initialize card reader.\n");
        while(1); // dead
    }
    pprintf("Ready for card swipe!\n");

    while(1)
    {
        sleep_ticks(100);
        TOG_GPIO(LED);

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
                pprintf("Got UID ");
                for (int i=0; i < got; i++) pprintf("%02X", uid[i]);
                pprintf("\n");
                break;

            // Error status is also normal and in real life would not report
            // it, but for this demo we'll be verbose.
            default:
                pprintf("Got error %d\n", got);
                break;
        }
    }
}
