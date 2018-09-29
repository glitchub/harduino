// LCD module support, for controller similar to Samsung KS6600, Hitachi
// HD44780, etc.    

#include "uno.h"    // uno pin definitions
#include "wait.h"   // waituS and waitmS

// Change these per your design. This code assumes write-only 4-bit mode, so
// the module's D0-D3 should be disconnected and RW should be tied low. 
#define D4 GPIO03   
#define D5 GPIO04
#define D6 GPIO05
#define D7 GPIO06
#define RS GPIO07 
#define E  GPIO08 

// Number of lines supported by module (must be 1 or 2)
#define LINES 2

#define DATA 0 // Send data (RS high) 
#define CMD4 1 // send 4-bit command only, from data high nibble
#define CMD8 2 // send 8-bit command
// send data to display in specified mode and delay 50uS
static void send(int mode, unsigned char data)
{
    if (!mode) PORT(RS) |= BIT(RS), mode=2; else PORT(RS) &= ~BIT(RS); // set register select
    for (int n=0; n < mode; n++, data<<=4)
    {
        PORT(E) |= BIT(E); 
        if (data & 0x10) PORT(D4) |= BIT(D4); else PORT(D4) &= ~BIT(D4);
        if (data & 0x20) PORT(D5) |= BIT(D5); else PORT(D5) &= ~BIT(D5);
        if (data & 0x40) PORT(D6) |= BIT(D6); else PORT(D6) &= ~BIT(D6);
        if (data & 0x80) PORT(D7) |= BIT(D7); else PORT(D7) &= ~BIT(D7);
        PORT(E) &= ~BIT(E);
    }
    waituS(40); 
}

void lcd_cls(void) 
{ 
    send(CMD8, 0x01); 
    waituS(1500);  // needs 1.52mS total
}

void lcd_goto(int col, int line)
{
    if (line >= LINES) return;
    if (col > ((LINES==1) ? 0x4f : 0x27)) return;
    send(CMD8, 0x80+(line*0x40)+col);
}    

void lcd_putc(char c)
{
    send(DATA, (unsigned)c);
}    

void lcd_puts(char *s)
{   
    while (*s) lcd_putc((unsigned)*s++);
}    

void lcd_init(void)
{
    waitmS(50); // allow display 50mS to come up

    DDR(D4) |= BIT(D4); DDR(D5) |= BIT(D5); DDR(D6) |= BIT(D6); DDR(D7) |= BIT(D7); DDR(RS) |= BIT(RS); DDR(E)  |= BIT(E);
    PORT(D4) &= ~BIT(D4); PORT(D5) &= ~BIT(D5); PORT(D6) &= ~BIT(D6); PORT(D7) &= ~BIT(D7); PORT(RS) &= ~BIT(RS); PORT(E)  &= ~BIT(E);

    // Assume the display did *not* just power on, so force a reset
    send(CMD4, 0x30);
    waituS(4100);
    send(CMD4, 0x30);
    waituS(100);
    send(CMD4, 0x30);
    
    // Initialize into 4-bit mode
    send(CMD4, 0x20);                       // set 4-bit interface 
    send(CMD8, (LINES==1) ? 0x20 : 0x28);   // function set: 4-bit, N lines, 5x8 
    send(CMD8, 0x0c);                       // display control: lcd on, cursor off, blink off 
    send(CMD8, 0x06);                       // entry mode set: cursor increment, no shift
    lcd_cls();                              
}    

