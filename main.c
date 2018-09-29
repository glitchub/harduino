#include <stdio.h>

#include "uno.h"
#include "lcd.h"
#include "ticks.h"

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

    DDR(GPIO13) |= BIT(GPIO13);                 // LED is attached to PIN13

    while(1)
    {
        unsigned long now=get_ticks()/1000;     // get current seconds
        if (now != then)                        // if different
        {
            then=now;                           // remember
            PIN(GPIO13) |= BIT(GPIO13);         // toggle the LED
            fprintf(lcd, "\f%ld\v\n%02d:%02d:%02d:%02d", now, (int)(now/86400), (int)((now % 86400)/3600), (int)((now % 3600)/60), (int)(now%60));
        }
    }
}


