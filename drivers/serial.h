// Serial port driver

// Stop serial port
void stop_serial(void); 

// (Re)start serial port at specified baud rate.
// If SERIAL_STDIO is defined, returns a FILE * that can be used with fprintf,
// etc. Note use of stdio will add about 1K to overall code size.
#ifdef SERIAL_STDIO
#include <stdio.h>
FILE *start_serial(uint32_t baud);
#else
void start_serial(uint32_t baud);
#endif

// Block until character can be written, then write it.
void write_serial(int8_t c);

// Return number of characters that can be written without blocking.
int8_t writeable_serial(void);
#define writable_serial writeable_serial // me no spel gud

// Block until character available, then return it
int8_t read_serial(void);

// Return number of characters that can be read without blocking.
int8_t readable_serial(void);
