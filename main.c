#include <stdio.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

#include "ticks.h"
#include "lcd.h"
#include "serial.h"
#include "nec.h"

#define D4 GPIO03
#define D5 GPIO04
#define D6 GPIO05
#define D7 GPIO06
#define RS GPIO07
#define E  GPIO08

// note use of fprintf increases code size by about 1kb

int main(void)
{
    unsigned long then=0;

    FILE *lcd = init_lcd(2, 16);                // 2x16 display
    fprintf(lcd, "Booting...");

    FILE *serial = init_serial(115200UL);

    cli();
    unsigned char exfuse = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    unsigned char lock = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    unsigned char hifuse = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    unsigned char lofuse = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    sei();

    fprintf(serial, "Fuses: lo=%02X hi=%02X ex=%02X lock=%02X\r\n", lofuse, hifuse, exfuse, lock);

    DDR(GPIO13) |= BIT(GPIO13);                 // LED is attached to PIN13

    start_ticks(0);                             // start free running clock
    start_nec();                                // start NEC ir receiver

    while(1)
    {
        unsigned long now=get_ticks()/1000;     // get current seconds
        if (now != then)                        // if different
        {
            then=now;                           // remember
            PIN(GPIO13) |= BIT(GPIO13);         // toggle the LED

            unsigned char d=now / 86400UL;
            unsigned char h=(now % 86400UL) / 3600;
            unsigned char m=(now % 3600) / 60;
            unsigned char s=now % 60;

            // For lcd, '\f'=go home, '\v'=clr to eol, '\n'=go to next line
            fprintf(lcd, "\f%ld\v\n%02u:%02u:%02u:%02u", now, d, h, m, s);

            // terminal requires \r\n
            fprintf(serial, "%ld %02u:%02u:%02u:%02u\r\n", now, d, h, m, s);

            if (readable_serial())
            {
                fprintf(serial, "Did you just say \"");
                do fputc(fgetc(serial), serial); while (readable_serial());
                fprintf(serial, "\"?\r\n");
            }
        }

        unsigned long key;
        if (get_nec(&key)==PRESSED) start_ticks(65530000);
    }
}
