// LCD module driver, for a controller similar to Samsung KS6600, Hitachi
// HD44780, etc.

// inline delay
#include "waituS.h"

// The module is operated in read-only 4-bit mode, the D0-D3 pins should be
// disconnected and RW should be tied low. (A, K and VO pins are
// hardware-specific, not in scope of this code). The following pins must be
// defined:
#if !defined(LCD_D4) || !defined(LCD_D5) || !defined(LCD_D6) || !defined(LCD_D7) || !defined(LCD_RS) || !defined(LCD_E)
#error "Must define LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_RS, and LCD_E"
#endif

// Send data to display in specified mode and delay for the nominal busy time.
// Note mode & 1 == send 2 nibbles, mode & 2 == assert RS
#define CMD4 0            // Send 4-bit command only, from data high nibble
#define CMD8 1            // Send 8-bit command
#define DATA 3            // Send data (RS high)
static void send(int8_t mode, uint8_t data)
{
    if (mode & 2) PORT(LCD_RS) |= BIT(LCD_RS); else PORT(LCD_RS) &= NOBIT(LCD_RS); // set register select
    for (int8_t n=0; n <= (mode & 1); n++, data<<=4)
    {
        PORT(LCD_E) |= BIT(LCD_E);
        if (data & 0x10) PORT(LCD_D4) |= BIT(LCD_D4); else PORT(LCD_D4) &= NOBIT(LCD_D4);
        if (data & 0x20) PORT(LCD_D5) |= BIT(LCD_D5); else PORT(LCD_D5) &= NOBIT(LCD_D5);
        if (data & 0x40) PORT(LCD_D6) |= BIT(LCD_D6); else PORT(LCD_D6) &= NOBIT(LCD_D6);
        if (data & 0x80) PORT(LCD_D7) |= BIT(LCD_D7); else PORT(LCD_D7) &= NOBIT(LCD_D7);
        PORT(LCD_E) &= NOBIT(LCD_E);
    }
    waituS(40);
}

// Display lines/columns, and current line/column
static uint8_t lines, columns, curl, curc;

static void set(int8_t l, int8_t c)
{
    curl=l;
    curc=c;
    if (curc < 0) curl--, curc=columns-1;
    if (curc >= columns) curl++, curc=0;
    if (curl < 0) curl = lines-1;
    if (curl >= lines) curl = 0;
    send(CMD8, 0x80 + (curl*0x40) + curc);
}

// write character to lcd, handle magic controls
void write_lcd(int8_t c)
{
    switch(c)
    {
        case '\b':              // backspace but not past first
            if (curc) set(curl, curc-1);
            break;

        case '\f':              // form-feed, home cursor
            set(0, 0);
            break;

        case '\n':              // line feed, advance to next line
            set(curl+1, 0);
            break;

        case '\r':              // carriage return, return to start of current line
            if (curc) set(curl, 0);
            break;

        case '\v':              // clear to end of line
            if (curc  == columns-1) break;
            for (int8_t n=curc; n < columns; n++) send(DATA, ' ');
            set(curl, curc);
            break;

        default:                // write char and advance
            if (curc == columns-1) break;
            send(DATA, c);
            curc++;
            break;
    }
}

// Given display lines and columns, initalize LCD. Requires ticks (with interrupts enabled).
#ifdef LCD_STDIO
static int put(char c, FILE *f) { (void)f; write_lcd(c); return 0; }
static FILE handle = FDEV_SETUP_STREAM(put, NULL, _FDEV_SETUP_WRITE);
FILE *init_lcd(uint8_t l, uint8_t c)
#else
void init_lcd(uint8_t l, uint8_t c)
#endif
{
    if (l < 1) l = 1; else if (l > 2) l = 2;
    if (c < 8) c = 8; else if (c > 40) c = 40;
    lines=l;
    columns=c;
    curc=curl=0;

    // Pins are outputs
    DDR(LCD_D4) |= BIT(LCD_D4);
    DDR(LCD_D5) |= BIT(LCD_D5);
    DDR(LCD_D6) |= BIT(LCD_D6);
    DDR(LCD_D7) |= BIT(LCD_D7);
    DDR(LCD_RS) |= BIT(LCD_RS);
    DDR(LCD_E)  |= BIT(LCD_E);

    sleep_ticks(50);                      // Allow display 50mS to come out of power-on reset

    // But assume the display did *not* just power on, so force a reset anyway
    send(CMD4, 0x30);
    waituS(4100);
    send(CMD4, 0x30);
    waituS(100);
    send(CMD4, 0x30);

    // Initialize into 4-bit mode
    send(CMD4, 0x20);                     // set 4-bit interface
    send(CMD8, (lines==1) ? 0x20 : 0x28); // function set: 4-bit, N lines, 5x8
    send(CMD8, 0x0c);                     // display control: lcd on, cursor off, blink off
    send(CMD8, 0x06);                     // entry mode set: cursor increment, no shift
    send(CMD8, 0x01);                     // clear display
    waituS(1500);                         // let clear take affect
#ifdef LCD_STDIO
    return &handle;
#endif
}
