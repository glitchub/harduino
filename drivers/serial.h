// Serial port driver

// Enable serial port for 115200 baud
void init_serial(void);

// Block or yield until character can be written, then write it.
void write_serial(int8_t c);

// Return true if characters can be written without blocking.
bool writeable_serial(void);
#define writable_serial writeable_serial // me no spel gud

// Block or yield until character available, then return it
int8_t read_serial(void);

// Return true if characters can be read without blocking.
bool readable_serial(void);

// Return pressed key, or -1 if none
int key_press(void);
