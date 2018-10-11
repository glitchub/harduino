// Harduino main demo program

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

#include "ticks.h"
#include "lcd.h"
#include "serial.h"
#include "nec.h"
#include "dht11.h"

// note use of fprintf increases code size by about 1kb
int main(void)
{
    cli();                                      // make sure interrupts are disabled 

    // get fuses
    uint8_t exfuse = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    uint8_t lock = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    uint8_t hifuse = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    uint8_t lofuse = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    
    // start various services
    FILE *serial = start_serial(115200UL);      // serial I/O
    FILE *lcd = start_lcd(2, 16);               // LCD module is 2x16
    start_nec();                                // NEC IR
    start_dht11();                              // Thermal/humidty sense
    start_ticks(0);                             // Start at tick 0

    DDR(GPIO13) |= BIT(GPIO13);                 // LED is attached to PIN13
    
    sei();                                      // Enable interrupts

    fprintf(lcd, "\fBooting...\n");             // \f == home cursor

    fprintf(serial, "Booting...\r\n");          // terminal always requires crlf
    fprintf(serial, "Fuses: lo=%02X hi=%02X ex=%02X lock=%02X\r\n", lofuse, hifuse, exfuse, lock);

    uint32_t then=0, input=0;

    while(1)
    {
        uint32_t now=get_ticks()/1000;          // get seconds
        if (now != then)                        // if different
        {
            then=now;                           // remember
            PIN(GPIO13) |= BIT(GPIO13);         // toggle the LED

            uint8_t d=now / 86400UL;
            uint8_t h=(now % 86400UL) / 3600;
            uint8_t m=(now % 3600) / 60;
            uint8_t s=now % 60;

            // For lcd, \f == home cursor, \v == clear to eol, \n == go to next line
            if (!input) fprintf(lcd, "\f%lu\v", now);
            fprintf(lcd, "\f\n%02u:%02u:%02u:%02u", d, h, m, s);

            fprintf(serial, "%ld %02u:%02u:%02u:%02u\r\n", now, d, h, m, s);

            if (readable_serial())
            {
                fprintf(serial, "Did you just say \"");
                do fputc(fgetc(serial), serial); while (readable_serial());
                fprintf(serial, "\"?\r\n");
            }

            // read dht every 2 seconds
            if (now & 1)
            {
                uint8_t dc, rh, r;
                r=read_dht11(&dc, &rh);
                if (r) fprintf(serial, "DHT error %d\r\n", r);
                else fprintf(serial, "Temp:%dC RH:%d%%\r\n", dc, rh); 
            }
        }

        uint32_t key;
        if (get_nec(&key)==NEC_PRESSED && NEC_VENDOR(key) == 0xff00)
        {
            int8_t n=-1;
            switch(NEC_EVENT(key))
            {
                case 0x45:                              // power
                    start_ticks(input);
                    input=0;
                    break;
                case 0x46: break;                       // vol+
                case 0x47: break;                       // stop
                case 0x44:                              // rew
                    input /= 10;
                    break;
                case 0x40: break;                       // play
                case 0x43: break;                       // ffwd
                case 0x07: break;                       // down
                case 0x15: break;                       // vol -
                case 0x09: break;                       // up
                case 0x16: n=0; break;                  // 0
                case 0x19: break;                       // eq
                case 0x0d: break;                       // rept
                case 0x0c: n=1; break;                  // 1
                case 0x18: n=2; break;                  // 2
                case 0x5e: n=3; break;                  // 3
                case 0x08: n=4; break;                  // 4
                case 0x1c: n=5; break;                  // 5
                case 0x5a: n=6; break;                  // 6
                case 0x42: n=7; break;                  // 7
                case 0x52: n=8; break;                  // 8
                case 0x4a: n=9; break;                  // 9
                default: fprintf(serial,"Unknown key %08lX", key); break;
            }
            if (n >= 0) input=(input*10)+n;
            fprintf(lcd, "\f%lu\xff\v", input);
        }
    }
}
