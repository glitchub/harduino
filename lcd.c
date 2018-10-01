// LCD module support, for controller similar to Samsung KS6600, Hitachi
// HD44780, etc.
#include <stdio.h>
#include <avr/io.h>
#include "wait.h"   // waituS and waitmS

// Change these per your design. This code assumes write-only 4-bit mode, so
// the module's D0-D3 should be disconnected and RW should be tied low.
#define D4 GPIO03
#define D5 GPIO04
#define D6 GPIO05
#define D7 GPIO06
#define RS GPIO07
#define E  GPIO08

// send data to display in specified mode and delay for the nominal busy time
#define DATA 0 // Send data (RS high)
#define CMD4 1 // send 4-bit command only, from data high nibble
#define CMD8 2 // send 8-bit command
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

// put character to lcd, handle magic controls
static int put(char c, FILE *f)
{
    (void) f;                   // f is unused

    switch(c)
    {
        case '\b':              // backspace but not past first
            if (column) set(line, column-1);
            break;

        case '\f':              // form-feed, home cursor
            set(0,0);
            break;

        case '\n':              // line feed, advance to next line
            set(line+1, 0);
            break;

        case '\r':              // carriage return, return to start of current line
            set(line, 0);
            break;

        case '\v':              // clear to end of line
            for (int n=column; n < columns; n++) send(DATA, ' ');
            set(line, column);
            break;

        default:
            send(DATA, c);      // write char and advance the cursor
            if (++column >= columns) set(line, columns-1); // but not past the end
            break;
    }
    return 0; // unconditional success
}

static FILE handle = FDEV_SETUP_STREAM(put, NULL, _FDEV_SETUP_WRITE);

// Given display lines and columns, return a pointer to a file handle for use with fprintf, etc.
FILE *init_lcd(int l, int c)
{
    if (l < 1) l = 1; if (l > 2) l = 2;
    if (c < 8) c = 8; if (c > 40) c = 40;
    lines=l; columns=c;

    // Configure outputs
    DDR(D4) |= BIT(D4); DDR(D5) |= BIT(D5); DDR(D6) |= BIT(D6); DDR(D7) |= BIT(D7); DDR(RS) |= BIT(RS); DDR(E)  |= BIT(E);
    PORT(D4) &= ~BIT(D4); PORT(D5) &= ~BIT(D5); PORT(D6) &= ~BIT(D6); PORT(D7) &= ~BIT(D7); PORT(RS) &= ~BIT(RS); PORT(E)  &= ~BIT(E);

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

    return &handle;
}
