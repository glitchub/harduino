// If set, start_lcd() returns a FILE* that can be used with fprintf, etc. Note
// use of stdio adds at least 1Kb to target image.
#define LCD_STDIO 1

// Change these per your design. The code assumes write-only 4-bit mode, so
// the module's D0-D3 should be disconnected and RW should be tied low.
#define LCD_D4 GPIO03
#define LCD_D5 GPIO04
#define LCD_D6 GPIO05
#define LCD_D7 GPIO06
#define LCD_RS GPIO07
#define LCD_E  GPIO02

// Initialize LCD module with visible columns and rows (line shifting not supported).
#if LCD_STDIO
FILE *start_lcd(int8_t columns, int8_t rows);
#else
void start_lcd(int8_t columns, int8_t rows);
#endif

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
