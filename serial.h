// (Re)initialialize serial port for specified baud rate and return static file
// handle usable with fpprintf, fgetc, etc.  
FILE *init_serial(unsigned long baud);

// Return number of characters that can be read or written from serial without
// blocking.
int readable_serial(void);
int writable_serial(void);

// meh
#define writeable_serial() writable_serial()

