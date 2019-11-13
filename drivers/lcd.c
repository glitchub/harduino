// LCD module driver, for a controller similar to Samsung KS6600, Hitachi
// HD44780, etc.

// Send data to display in specified mode and delay for the nominal busy time.
// Note mode & 1 == send 2 nibbles, mode & 2 == assert RS
#define CMD4 0            // Send 4-bit command only, from data high nibble
#define CMD8 1            // Send 8-bit command
#define DATA 3            // Send data (RS high)
static void send(lcd *l, int8_t mode, uint8_t data)
{
    if (mode & 2) set_gpio(l->RS); else clr_gpio(l->RS); // set register select
    for (int8_t n=0; n <= (mode & 1); n++, data<<=4)
    {
        set_gpio(l->E);
        if (data & 0x10) set_gpio(l->D4); else clr_gpio(l->D4);
        if (data & 0x20) set_gpio(l->D5); else clr_gpio(l->D5);
        if (data & 0x40) set_gpio(l->D6); else clr_gpio(l->D6);
        if (data & 0x80) set_gpio(l->D7); else clr_gpio(l->D7);
        clr_gpio(l->E);
    }
    waituS(40);
}

// set lcd line and column
static void set(lcd *l, int8_t line, int8_t column)
{
    // wrap, etc
    if (column < 0) line--, column=l->columns-1;
    if (column >= l->columns) line++, column=0;
    if (line < 0) line = l->lines-1;
    if (line >= l->lines) line = 0;
    // set position
    send(l, CMD8, 0x80 + (line*0x40) + column);
    // and remember it
    l->curl=line;
    l->curc=column;
}

// write character to lcd, handle magic controls
void write_lcd(lcd *l, int8_t c)
{
    switch(c)
    {
        case '\b':              // backspace but not past first
            if (l->curc) set(l, l->curl, l->curc-1);
            break;

        case '\f':              // form-feed, home cursor
            set(l, 0, 0);
            break;

        case '\n':              // line feed, advance to next line
            set(l, l->curl+1, 0);
            break;

        case '\r':              // carriage return, return to start of current line
            if (l->curc) set(l, l->curl, 0);
            break;

        case '\v':              // clear to end of line
            if (l->curc  == l->columns-1) break;
            for (int8_t n=l->curc; n < l->columns; n++) send(l, DATA, ' ');
            set(l, l->curl, l->curc);
            break;

        default:                // write char and advance
            if (l->curc == l->columns-1) break;
            send(l, DATA, c);
            l->curc++;
            break;
    }
}

#ifdef LCD_STDIO
// put character to LCD via handle
static int put(char c, FILE *f) { write_lcd(f->udata, c); return 0; }
#endif

// initialize lcd, it must define gpios, columns, and lines
void init_lcd(lcd *l)
{
    // sanity
    if (l->lines < 1) l->lines = 1; else if (l->lines > 2) l->lines = 2;
    if (l->columns < 8) l->columns = 8; else if (l->columns > 40) l->columns = 40;
    l->curc=l->curl=0;

    // Pins are outputs
    out_gpio(l->D4);
    out_gpio(l->D5);
    out_gpio(l->D6);
    out_gpio(l->D7);
    out_gpio(l->RS);
    out_gpio(l->E);

    sleep_ticks(50);                            // Allow display 50mS to come out of power-on reset

    // But assume the display did *not* just power on, so force a reset anyway
    send(l, CMD4, 0x30);
    waituS(4100);
    send(l, CMD4, 0x30);
    waituS(100);
    send(l, CMD4, 0x30);

    // Initialize into 4-bit mode
    send(l, CMD4, 0x20);                        // set 4-bit interface
    send(l, CMD8, (l->lines==1) ? 0x20 : 0x28); // function set: 4-bit, N lines, 5x8
    send(l, CMD8, 0x0c);                        // display control: lcd on, cursor off, blink off
    send(l, CMD8, 0x06);                        // entry mode set: cursor increment, no shift
    send(l, CMD8, 0x01);                        // clear display
    waituS(1500);                               // let clear take affect

#ifdef LCD_STDIO
    // prepare handle
    l->handle.put = put;
    l->handle.get = NULL;
    l->handle.flags = _FDEV_SETUP_WRITE;
    l->handle.udata = l;                        // so put() can find the struct
#endif
}
