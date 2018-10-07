// If set, init_serial() returns a file handle that can be used with fprintf,
// fgets, etc. Note use of stdio adds at least 1Kb to target image.
#define SERIAL_STDIO 1

// Set these as desired to control how much ram will be allocated for transmit
// and receive (+2 bytes). Maximum value is 256. Set to 0 to disable the
// corresponding tx or rx write functions completely. 
#define SERIAL_TX_SIZE 60  // transmit buffer size
#define SERIAL_RX_SIZE 4   // receive buffer size

// (Re)initialialize serial port for specified baud rate.
#if SERIAL_STDIO
#include <stdio.h>
FILE *init_serial(unsigned long baud);
#else
void init_serial(unsigned long baud);
#endif

// Block until character can be written, then write it.
void write_serial(char c);

// Return number of characters that can be written without blocking.
int writeable_serial(void);
#define writable_serial writeable_serial // me no spel gud

// Block until character available, then return it
char read_serial(void);

// Return number of characters that can be read without blocking.
int readable_serial(void);
