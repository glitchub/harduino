// LCD module driver, for a controller similar to Samsung KS6600, Hitachi
// HD44780, etc.  

// Write character to LCD and move cursor right, not past the end of the
// line. Note the dispaly does not scroll.
//
// The following characters have special behavior:
//   \b - move cursor left one column, but not past the start of the line
//   \f - move cursor to first column of first line
//   \n - move cursor to first column of next line, wrap back to first line
//   \r - move cursor to first column of current line
//   \v - clear text to end of line
void write_lcd(int8_t c);
// Disable the driver, restore all pins to inputs. Does not change current
// state of the LCD.
void stop_lcd(void);

// Initialize LCD module with visible columns and rows (line shifting is not
// supported).
#ifdef LCD_STDIO
// If LCD_STDIO is defined then start_lcd() returns a FILE* that can be used
// with fprintf, etc. Note the use of stdio adds at least 1Kb to the target
// image.
#include <stdio.h>
FILE *start_lcd(uint8_t lines, uint8_t columns);
#else
void start_lcd(uint8_t lines, uint8_t columns);
#endif

