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

    lcd_init();
    lcd_puts("Hello!");
    
    DDR(GPIO13) |= BIT(GPIO13);                 // LED is attached to PIN13

    while(1)
    {   
        unsigned long now=get_ticks()/1000;     // get current seconds
        if (now != then)                        // if different
        {
            then=now;                           // remember
            PIN(GPIO13) |= BIT(GPIO13);         // toggle the LED

            lcd_goto(0,1);                      // print time on line 2
            char t[16];                      
            sprintf(t, "%02d:%02d:%02d:%02d", (int)(now/86400), (int)((now % 86400)/3600), (int)((now % 3600)/60), (int)(now%60));
            lcd_puts(t);
        }    
    }
}


