// Serial port driver

// Enable serial port at specified baud rate.
// If SERIAL_STDIO is defined, returns a FILE * that can be used with fprintf,
// etc. Note use of stdio will add about 1K to overall code size.
#ifdef SERIAL_STDIO
#include <stdio.h>
FILE *init_serial(uint32_t baud);
#else
void init_serial(uint32_t baud);
#endif

// Block or yield until character can be written, then write it.
void write_serial(int8_t c);

// Return true if characters can be written without blocking.
char writeable_serial(void);
#define writable_serial writeable_serial // me no spel gud

// Block or yield until character available, then return it
int8_t read_serial(void);

// Return true if characters can be read without blocking.
char readable_serial(void);
