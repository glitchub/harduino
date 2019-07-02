// LCD module driver, for a controller similar to Samsung KS6600, Hitachi
// HD44780, etc.
#ifndef LCD_H
#define LCD_H

#include <stdio.h>

// if defined, support fprintf(lcd->handle,...) etc 
#define LCD_STDIO

typedef struct
{
    gpio *E, *RS, *D4, *D5, *D6, *D7;   // gpios for 6 pins
    int8_t lines, columns;              // max lines and columns
    int8_t curl, curc;                  // current line and column
#ifdef LCD_STDIO
    FILE handle;                        // file handle for stdio
#endif
} lcd;

// Initialize LCD module with defined data structure
void init_lcd(lcd *l);

// Write character to LCD and move cursor right, not past the end of the
// line. Note the dispaly does not scroll.
//
// The following characters have special behavior:
//   \b - move cursor left one column, but not past the start of the line
//   \f - move cursor to first column of first line
//   \n - move cursor to first column of next line, wrap back to first line
//   \r - move cursor to first column of current line
//   \v - clear text to end of line
void write_lcd(lcd *l, int8_t c);
#endif
