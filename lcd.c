// LCD module support, for controller similar to Samsung KS6600, Hitachi
// HD44780, etc.

#include <stdio.h>
#include <avr/io.h>
#include "wait.h"   // waituS and waitmS
#include "lcd.h"

// send data to display in specified mode and delay for the nominal busy time
#define DATA 0 // Send data (RS high)
#define CMD4 1 // send 4-bit command only, from data high nibble
#define CMD8 2 // send 8-bit command
static void send(int mode, unsigned char data)
{
    if (!mode) PORT(LCD_RS) |= BIT(LCD_RS), mode=2; else PORT(LCD_RS) &= ~BIT(LCD_RS); // set register select
    for (int n=0; n < mode; n++, data<<=4)
    {
        PORT(LCD_E) |= BIT(LCD_E);
        if (data & 0x10) PORT(LCD_D4) |= BIT(LCD_D4); else PORT(LCD_D4) &= ~BIT(LCD_D4);
        if (data & 0x20) PORT(LCD_D5) |= BIT(LCD_D5); else PORT(LCD_D5) &= ~BIT(LCD_D5);
        if (data & 0x40) PORT(LCD_D6) |= BIT(LCD_D6); else PORT(LCD_D6) &= ~BIT(LCD_D6);
        if (data & 0x80) PORT(LCD_D7) |= BIT(LCD_D7); else PORT(LCD_D7) &= ~BIT(LCD_D7);
        PORT(LCD_E) &= ~BIT(LCD_E);
    }
    waituS(40);
}

// max and current column and line
static int columns, lines, column, line;

static void set(int l, int c)
{
    line=l;
    column=c;
    if (column < 0) line--, column = columns-1;
    if (column >= columns) line++, column = 0;
    if (line < 0) line = lines-1;
    if (line >= lines) line = 0;
    send(CMD8, 0x80 + (line*0x40) + column);
}

// write character to lcd, handle magic controls
void write_lcd(char c)
{
    switch(c)
    {
        case '\b':              // backspace but not past first
            if (column) set(line, column-1);
            break;

        case '\f':              // form-feed, home cursor
            set(0, 0);
            break;

        case '\n':              // line feed, advance to next line
            set(line+1, 0);
            break;

        case '\r':              // carriage return, return to start of current line
            if (column) set(line, 0);
            break;

        case '\v':              // clear to end of line
            if (column == columns-1) break;
            for (int n=column; n < columns; n++) send(DATA, ' ');
            set(line, column);
            break;

        default:                // write char and advance
            if (column == columns-1) break;
            send(DATA, c);
            column++;
            break;
    }
}

// Given display lines and columns, initalize LCD.
#if LCD_STDIO
static int put(char c, FILE *f) { (void)f; write_lcd(c); return 0; }
static FILE handle = FDEV_SETUP_STREAM(put, NULL, _FDEV_SETUP_WRITE);
FILE *init_lcd(int l, int c)
#else
void init_lcd(int l, int c)
#endif
{
    if (l < 1) l = 1; if (l > 2) l = 2;
    if (c < 8) c = 8; if (c > 40) c = 40;
    lines=l; columns=c;

    // Configure outputs
    DDR(LCD_D4) |= BIT(LCD_D4);
    DDR(LCD_D5) |= BIT(LCD_D5);
    DDR(LCD_D6) |= BIT(LCD_D6);
    DDR(LCD_D7) |= BIT(LCD_D7);
    DDR(LCD_RS) |= BIT(LCD_RS);
    DDR(LCD_E)  |= BIT(LCD_E);
    PORT(LCD_D4) &= ~BIT(LCD_D4);
    PORT(LCD_D5) &= ~BIT(LCD_D5);
    PORT(LCD_D6) &= ~BIT(LCD_D6);
    PORT(LCD_D7) &= ~BIT(LCD_D7);
    PORT(LCD_RS) &= ~BIT(LCD_RS);
    PORT(LCD_E)  &= ~BIT(LCD_E);

    waitmS(50); // Allow display 50mS to come out of power-on reset

    // But assume the display did *not* just power on, so force a reset anyway
    send(CMD4, 0x30);
    waituS(4100);
    send(CMD4, 0x30);
    waituS(100);
    send(CMD4, 0x30);

    // Initialize into 4-bit mode
    send(CMD4, 0x20);                       // set 4-bit interface
    send(CMD8, (lines==1) ? 0x20 : 0x28);   // function set: 4-bit, N lines, 5x8
    send(CMD8, 0x0c);                       // display control: lcd on, cursor off, blink off
    send(CMD8, 0x06);                       // entry mode set: cursor increment, no shift
    send(CMD8, 0x01);                       // clear display
    waituS(1500);                           // let clear take affect
    column=line=0;
#if LCD_STDIO
    return &handle;
#endif
}
