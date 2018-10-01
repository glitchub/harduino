#include <stdio.h>
#include <avr/io.h>
#include "lcd.h"
#include "ticks.h"
#include "serial.h"

#define D4 GPIO03
#define D5 GPIO04
#define D6 GPIO05
#define D7 GPIO06
#define RS GPIO07
#define E  GPIO08

int main(void)
{
    unsigned long then=0;

    start_ticks();                              // start free running clock

    FILE *lcd = init_lcd(2, 16);                // 2x16 display
    fprintf(lcd, "Hello there!");

    FILE *serial = init_serial(115200UL);
    fprintf(serial, "Hello from serial!\r\n");

    DDR(GPIO13) |= BIT(GPIO13);                 // LED is attached to PIN13

    while(1)
    {
        unsigned long now=get_ticks()/1000;     // get current seconds
        if (now != then)                        // if different
        {
            then=now;                           // remember
            PIN(GPIO13) |= BIT(GPIO13);         // toggle the LED

            int d=(int)(now/86400);
            int h=(int)(now % 86400)/3600;
            int m=(int)(now % 3600)/60;
            int s=(int)(now % 60);

            // For lcd. \f=go to first line, \v=clr to eol, \n=go to next line
            fprintf(lcd, "\f%ld\v\n%02d:%02d:%02d:%02d", now, d, h, m, s);

            fprintf(serial, "%ld -> %02d:%02d:%02d:%02d\r\n", now, d, h, m, s);

            if (readable_serial())
            {
                fprintf(serial, "Did you just say '");
                do fputc(fgetc(serial), serial); while (readable_serial());
                fprintf(serial, "'?\r\n");
            }
        }
    }
}
