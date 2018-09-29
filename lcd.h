// Initialize LCD module and return a file handle to be used with fputc, puts,
// fprintf, etc. Specify visible columns and rows, line shifting is not
// supported.

FILE *init_lcd(int columns, int rows);

// Printing a character moves the cursor right, but not past the end of the
// line.
//
// The following characters have special behavior:
//
//   \b - move cursor left, but not past the start of the line
//   \f - move cursor to first column of first line
//   \n - move cursor to first column of next line, will wrap back to first line.
//   \r - move cursor to first column of current line
//   \v - clear text to end of line
//
// Note the display does not scroll.
